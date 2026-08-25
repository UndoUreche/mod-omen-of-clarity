/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"
#include "DatabaseEnv.h"
#include "DBCStores.h"

#include <mutex>
#include <unordered_set>

static constexpr uint8 MOD_LOCKED_GLYPH_SLOT = 5;
static constexpr uint32 MOD_LOCKED_SLOT_BIT = 0x20;

enum ClassicGlyphsModSpells
{
    SPELL_OMEN_OF_CLARITY = 16864,
    SPELL_CLEARCASTING = 16870
};

enum ClassicGlyphsModMisc
{
    BOOK_TEXT_MOD_ENABLED = 90001,
    BOOK_TEXT_MOD_DISABLED = 90002,

    BOOK_GOBJECT_ID = 42325,

    BOOK_DISABLED_NPC_TEXT = 42327,
    BOOK_DRUID_NPC_TEXT = 42328,
    BOOK_PALADIN_NPC_TEXT = 42329,
    BOOK_HUNTER_NPC_TEXT = 42330,

    BOOK_GOSSIP_MENU_ID = 90001,
    BOOK_GOSSIP_SENDER = 80000,

    BOOK_GOSSIP_ACTION_INSPECT = 0,
    BOOK_GOSSIP_ACTION_PURGE = 1
};

// =====================================================
// In-memory cache of player GUIDs with feature enabled
// =====================================================
static std::mutex s_modLock;
static std::unordered_map<uint32, uint8> s_modEnabled;

static bool IsModEnabled(uint32 guidLow, uint8 spec)
{
    std::lock_guard<std::mutex> lock(s_modLock);
    return s_modEnabled.contains(guidLow) && (s_modEnabled.at(guidLow) & 1 << spec) > 0;
}

static bool IsModEnabled(Player *player)
{
    return IsModEnabled(player->GetGUID().GetCounter(), player->GetActiveSpec());
}

static void SetModMask(uint32 guidLow, uint8 mask)
{
    std::lock_guard<std::mutex> lock(s_modLock);

    s_modEnabled.insert_or_assign(guidLow, mask);
}

static uint8 GetModMask(uint32 guidLow)
{
    if (s_modEnabled.contains(guidLow))
        return s_modEnabled.at(guidLow);

    return 0;
}

static void SetModEnabled(uint32 guidLow, uint8 spec, bool enabled)
{
    std::lock_guard<std::mutex> lock(s_modLock);

    uint8 currentMask = 0;
    if (s_modEnabled.contains(guidLow))
        currentMask = s_modEnabled.at(guidLow);

    uint8 newMask = 1 << spec;
    if (enabled)
        newMask = currentMask | newMask;
    else
        newMask = currentMask & ~newMask;

    if (newMask == 0)
        s_modEnabled.erase(guidLow);
    else
        s_modEnabled.insert_or_assign(guidLow, newMask);
}

static void SetModEnabled(Player *player, bool enabled)
{
    return SetModEnabled(player->GetGUID().GetCounter(), player->GetActiveSpec(), enabled);
}

// =====================================================
// Glyph slot locking helpers
// =====================================================
static void RemoveGlyphFromSlot(Player *player, uint8 slot)
{
    uint32 glyph = player->GetGlyph(slot);
    if (!glyph)
        return;

    GlyphPropertiesEntry const *glyphEntry =
            sGlyphPropertiesStore.LookupEntry(glyph);
    if (!glyphEntry)
        return;

    player->RemoveAurasDueToSpell(glyphEntry->SpellId);

    Unit::AuraMap &ownedAuras = player->GetOwnedAuras();
    for (auto iter = ownedAuras.begin(); iter != ownedAuras.end();)
    {
        Aura *aura = iter->second;
        if (SpellInfo const *triggeredBy =
                aura->GetTriggeredByAuraSpellInfo())
        {
            if (triggeredBy->Id == glyphEntry->SpellId)
            {
                player->RemoveOwnedAura(iter);
                continue;
            }
        }
        ++iter;
    }

    player->SendLearnPacket(glyphEntry->SpellId, false);
    player->SetGlyph(slot, 0, true);
    player->SendTalentsInfoData(false);
}

static void EnsureGlyphSlotLocked(Player *player)
{
    uint32 bits = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
    if (bits & MOD_LOCKED_SLOT_BIT)
    {
        player->SetUInt32Value(PLAYER_GLYPHS_ENABLED,
                               bits & ~MOD_LOCKED_SLOT_BIT);
        player->SendTalentsInfoData(false);
    }
}

static void LockGlyphSlot(Player *player)
{
    RemoveGlyphFromSlot(player, MOD_LOCKED_GLYPH_SLOT);

    // Clear slot 5 glyph from inactive spec to prevent it from being
    // applied during ActivateSpec before our hook can strip it.
    // glyph6 = slot index 5 in character_glyphs table.
    CharacterDatabase.Execute(
        "UPDATE character_glyphs SET glyph6 = 0 WHERE guid = {}",
        player->GetGUID().GetCounter());

    uint32 bits = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
    player->SetUInt32Value(PLAYER_GLYPHS_ENABLED,
                           bits & ~MOD_LOCKED_SLOT_BIT);
    player->SendTalentsInfoData(false);
}

static void UnlockGlyphSlot(Player *player)
{
    if (player->GetLevel() >= 80)
    {
        uint32 bits = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
        player->SetUInt32Value(PLAYER_GLYPHS_ENABLED,
                               bits | MOD_LOCKED_SLOT_BIT);
        player->SendTalentsInfoData(false);
    }
}

// =====================================================
// 16857 - Faerie Fire (Feral) proc Clearcasting
// =====================================================
class spell_ooc_faerie_fire_feral : public SpellScript
{
    PrepareSpellScript(spell_ooc_faerie_fire_feral);

    bool Validate(SpellInfo const * /*spellInfo*/) override
    {
        return ValidateSpellInfo({SPELL_OMEN_OF_CLARITY, SPELL_CLEARCASTING});
    }

    void HandleAfterHit()
    {
        Unit *caster = GetCaster();
        Unit *target = GetHitUnit();

        if (!caster || !target)
            return;

        Player *player = caster->ToPlayer();
        if (!player)
            return;

        if (!IsModEnabled(player))
            return;

        if (!player->HasAura(SPELL_OMEN_OF_CLARITY))
            return;

        if (target->GetTypeId() == TYPEID_PLAYER)
            return;

        if (Creature *creature = target->ToCreature())
            if (creature->IsPet() && creature->GetOwner() &&
                creature->GetOwner()->GetTypeId() == TYPEID_PLAYER)
                return;

        player->CastSpell(player, SPELL_CLEARCASTING, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_ooc_faerie_fire_feral::HandleAfterHit);
    }
};

// =====================================================
// Player scripts: login cache + quest chain completion
// =====================================================
class classic_glyph_player_script : public PlayerScript
{
public:
    classic_glyph_player_script() : PlayerScript("classic_glyph_player_script",
                                          {
                                              PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT,
                                              PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST,
                                              PLAYERHOOK_ON_LEVEL_CHANGED,
                                              PLAYERHOOK_ON_AFTER_SPEC_SLOT_CHANGED,
                                              PLAYERHOOK_CAN_CAST_ITEM_USE_SPELL
                                          })
    {
    }

    void OnPlayerLogin(Player *player) override
    {
        uint32 guidLow = player->GetGUID().GetCounter();
        QueryResult result = CharacterDatabase.Query(
            "SELECT spec FROM mod_classic_glyphs_enabled WHERE guid = {}",
            guidLow);

        if (result)
        {
            const Field *fields = result->Fetch();
            const uint8 specMask = fields[0].Get<uint8>();

            SetModMask(guidLow, specMask);

            if (IsModEnabled(player)) //lock only if enabled on the active spec
                LockGlyphSlot(player);
        }
    }

    void OnPlayerLogout(Player *player) override
    {
        SetModEnabled(player, false);
    }

    void OnPlayerLevelChanged(Player *player, uint8 /*oldLevel*/) override
    {
        if (IsModEnabled(player))
            EnsureGlyphSlotLocked(player);
    }

    void OnPlayerAfterSpecSlotChanged(Player *player,
                                      uint8 /*newSlot*/) override
    {
        if (IsModEnabled(player))
        {
            RemoveGlyphFromSlot(player, MOD_LOCKED_GLYPH_SLOT);
            EnsureGlyphSlotLocked(player);
        } else
            UnlockGlyphSlot(player);
    }

    bool OnPlayerCanCastItemUseSpell(Player *player, Item * /*item*/,
                                     SpellCastTargets const & /*targets*/,
                                     uint8 /*castCount*/,
                                     uint32 glyphIndex) override
    {
        if (glyphIndex == MOD_LOCKED_GLYPH_SLOT && IsModEnabled(player))
            return false;
        return true;
    }
};

class go_classic_glyph_book : public GameObjectScript
{
    static bool modAvailableForClass(uint32 playerClass)
    {
        return playerClass == CLASS_DRUID
               || playerClass == CLASS_HUNTER
               || playerClass == CLASS_PALADIN;
    }

public:
    go_classic_glyph_book() : GameObjectScript("go_classic_glyph_book")
    {
    }

    bool OnGossipHello(Player *player, GameObject *gameobject) override
    {
        // SendGossipMenuFor(player, player->GetGossipTextId(gameobject), gameobject->GetGUID());
        // return true;

        if (gameobject->GetEntry() != BOOK_GOBJECT_ID)
            return false;

        // Build the normal gossip menu (DB options + quest icons)
        player->PrepareGossipMenu(gameobject, DEFAULT_GOSSIP_MESSAGE, true);

        uint32 textId;

        if (const uint32 playerClass = player->getClass(); modAvailableForClass(playerClass))
        {
            switch (playerClass)
            {
                case CLASS_DRUID:
                    textId = BOOK_DRUID_NPC_TEXT;
                    break;
                case CLASS_HUNTER:
                    textId = BOOK_HUNTER_NPC_TEXT;
                    break;
                default:
                    textId = BOOK_PALADIN_NPC_TEXT;
                    break;
            }

            const uint32 optionId = IsModEnabled(player)
                                        ? BOOK_GOSSIP_ACTION_PURGE
                                        : BOOK_GOSSIP_ACTION_INSPECT;

            AddGossipItemFor(player, BOOK_GOSSIP_MENU_ID, optionId, BOOK_GOSSIP_SENDER, optionId);
        } else
        {
            textId = BOOK_DISABLED_NPC_TEXT;
        }

        SendGossipMenuFor(player, textId, gameobject->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player *player, GameObject *gameObject, uint32 sender, uint32 action) override
    {
        if (gameObject->GetEntry() != BOOK_GOBJECT_ID)
            return false;

        if (sender != BOOK_GOSSIP_SENDER)
            return false;

        uint32 guidLow = player->GetGUID().GetCounter();
        uint32 responseId;

        if (action == BOOK_GOSSIP_ACTION_INSPECT)
        {
            SetModEnabled(player, true);

            CharacterDatabase.Execute(
                "INSERT IGNORE INTO mod_classic_glyphs_enabled (guid, spec) VALUES ({}, {})",
                guidLow, GetModMask(guidLow));

            LockGlyphSlot(player);
            responseId = BOOK_TEXT_MOD_ENABLED;
        } else
        {
            SetModEnabled(player, false);

            if (GetModMask(guidLow) == 0)
                CharacterDatabase.Execute(
                    "DELETE FROM mod_classic_glyphs_enabled WHERE guid = {}",
                    guidLow);

            UnlockGlyphSlot(player);
            responseId = BOOK_TEXT_MOD_DISABLED;
        }

        ClearGossipMenuFor(player);
        SendGossipMenuFor(player, responseId, gameObject->GetOwnerGUID());

        return true;
    }
};

void AddClassicGlyphScripts()
{
    RegisterSpellScript(spell_ooc_faerie_fire_feral);
    new classic_glyph_player_script();
    new go_classic_glyph_book();
}