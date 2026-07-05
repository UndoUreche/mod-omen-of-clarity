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

static constexpr uint8  OOC_LOCKED_GLYPH_SLOT = 5;
static constexpr uint32 OOC_LOCKED_SLOT_BIT   = 0x20;

enum OmenOfClaritySpells
{
    SPELL_OMEN_OF_CLARITY = 16864,
    SPELL_CLEARCASTING    = 16870
};

enum OmenOfClarityMisc
{
    NPC_KEEPER_REMULOS            = 11832,
    QUEST_A_DEEPER_CONNECTION     = 100005,
    REMULOS_GOSSIP_TEXT           = 14198,
    OOC_GOSSIP_MENU_ID            = 90001,
    OOC_GOSSIP_OPTION_ENABLE      = 0,
    OOC_GOSSIP_OPTION_DISABLE     = 1,
    OOC_GOSSIP_SENDER             = 50000,
    OOC_GOSSIP_ACTION_TOGGLE      = 1,
    NPC_TEXT_OOC_ENABLED          = 90001,
    NPC_TEXT_OOC_DISABLED         = 90002
};

// =====================================================
// In-memory cache of player GUIDs with feature enabled
// =====================================================
static std::mutex s_oocLock;
static std::unordered_map<uint32, uint8> s_oocEnabled;

static bool IsOocFfEnabled(uint32 guidLow, uint8 spec)
{
    std::lock_guard<std::mutex> lock(s_oocLock);
    return s_oocEnabled.contains(guidLow) && (s_oocEnabled.at(guidLow) & 1 << spec) > 0;
}

static bool IsOocFfEnabled(Player* player)
{
    return IsOocFfEnabled(player->GetGUID().GetCounter(), player->GetActiveSpec());
}

static void SetOocFfMask(uint32 guidLow, uint8 mask)
{
    std::lock_guard<std::mutex> lock(s_oocLock);

    s_oocEnabled.insert_or_assign(guidLow, mask);
}

static uint8 GetOocFfMask(uint32 guidLow)
{
    if (s_oocEnabled.contains(guidLow))
        return s_oocEnabled.at(guidLow);

    return 0;
}

static void SetOocFfEnabled(uint32 guidLow, uint8 spec, bool enabled)
{
    std::lock_guard<std::mutex> lock(s_oocLock);

    uint8 currentMask = 0;
    if (s_oocEnabled.contains(guidLow))
        currentMask = s_oocEnabled.at(guidLow);

    uint8 newMask = 1 << spec;
    if (enabled)
        newMask = currentMask | newMask;
    else
        newMask = currentMask & ~newMask;

    if (newMask == 0)
        s_oocEnabled.erase(guidLow);
    else
        s_oocEnabled.insert_or_assign(guidLow, newMask);
}

static void SetOocFfEnabled(Player* player, bool enabled)
{
    return SetOocFfEnabled(player->GetGUID().GetCounter(), player->GetActiveSpec(), enabled);
}

// =====================================================
// Glyph slot locking helpers
// =====================================================
static void RemoveGlyphFromSlot(Player* player, uint8 slot)
{
    uint32 glyph = player->GetGlyph(slot);
    if (!glyph)
        return;

    GlyphPropertiesEntry const* glyphEntry =
        sGlyphPropertiesStore.LookupEntry(glyph);
    if (!glyphEntry)
        return;

    player->RemoveAurasDueToSpell(glyphEntry->SpellId);

    Unit::AuraMap& ownedAuras = player->GetOwnedAuras();
    for (auto iter = ownedAuras.begin(); iter != ownedAuras.end();)
    {
        Aura* aura = iter->second;
        if (SpellInfo const* triggeredBy =
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

static void EnsureGlyphSlotLocked(Player* player)
{
    uint32 bits = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
    if (bits & OOC_LOCKED_SLOT_BIT)
    {
        player->SetUInt32Value(PLAYER_GLYPHS_ENABLED,
                               bits & ~OOC_LOCKED_SLOT_BIT);
        player->SendTalentsInfoData(false);
    }
}

static void LockGlyphSlot(Player* player)
{
    RemoveGlyphFromSlot(player, OOC_LOCKED_GLYPH_SLOT);

    // Clear slot 5 glyph from inactive spec to prevent it from being
    // applied during ActivateSpec before our hook can strip it.
    // glyph6 = slot index 5 in character_glyphs table.
    CharacterDatabase.Execute(
        "UPDATE character_glyphs SET glyph6 = 0 WHERE guid = {}",
        player->GetGUID().GetCounter());

    uint32 bits = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
    player->SetUInt32Value(PLAYER_GLYPHS_ENABLED,
                           bits & ~OOC_LOCKED_SLOT_BIT);
    player->SendTalentsInfoData(false);
}

static void UnlockGlyphSlot(Player* player)
{
    if (player->GetLevel() >= 80)
    {
        uint32 bits = player->GetUInt32Value(PLAYER_GLYPHS_ENABLED);
        player->SetUInt32Value(PLAYER_GLYPHS_ENABLED,
                               bits | OOC_LOCKED_SLOT_BIT);
        player->SendTalentsInfoData(false);
    }
}

// =====================================================
// 16857 - Faerie Fire (Feral) proc Clearcasting
// =====================================================
class spell_ooc_faerie_fire_feral : public SpellScript
{
    PrepareSpellScript(spell_ooc_faerie_fire_feral);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_OMEN_OF_CLARITY, SPELL_CLEARCASTING });
    }

    void HandleAfterHit()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();

        if (!caster || !target)
            return;

        Player* player = caster->ToPlayer();
        if (!player)
            return;

        if (!IsOocFfEnabled(player))
            return;

        if (!player->HasAura(SPELL_OMEN_OF_CLARITY))
            return;

        if (target->GetTypeId() == TYPEID_PLAYER)
            return;

        if (Creature* creature = target->ToCreature())
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
class ooc_ff_player_script : public PlayerScript
{
public:
    ooc_ff_player_script() : PlayerScript("ooc_ff_player_script",
        {PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LOGOUT,
         PLAYERHOOK_ON_PLAYER_COMPLETE_QUEST,
         PLAYERHOOK_ON_LEVEL_CHANGED,
         PLAYERHOOK_ON_AFTER_SPEC_SLOT_CHANGED,
         PLAYERHOOK_CAN_CAST_ITEM_USE_SPELL}) { }

    void OnPlayerLogin(Player* player) override
    {
        uint32 guidLow = player->GetGUID().GetCounter();
        QueryResult result = CharacterDatabase.Query(
            "SELECT spec FROM mod_ooc_ff_enabled WHERE guid = {}",
            guidLow);

        if (result)
        {
            const Field* fields = result->Fetch();
            const uint8 specMask = fields[0].Get<uint8>();

            SetOocFfMask(guidLow, specMask);

            if (IsOocFfEnabled(player)) //lock only if enabled on the active spec
                LockGlyphSlot(player);
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        SetOocFfEnabled(player, false);
    }

    void OnPlayerCompleteQuest(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_A_DEEPER_CONNECTION)
            return;

        uint32 guidLow = player->GetGUID().GetCounter();
        uint8 spec = player->GetActiveSpec();

        SetOocFfEnabled(player, true);
        LockGlyphSlot(player);

        CharacterDatabase.Execute(
            "INSERT IGNORE INTO mod_ooc_ff_enabled (guid, spec) VALUES ({}, {})",
            guidLow, GetOocFfMask(guidLow));

        if (Creature* remulos = player->FindNearestCreature(
                NPC_KEEPER_REMULOS, 50.0f))
        {
            remulos->SendPlaySpellVisual(179);
            remulos->SendPlaySpellImpact(player->GetGUID(), 362);
        }
    }

    void OnPlayerLevelChanged(Player* player, uint8 /*oldLevel*/) override
    {
        if (IsOocFfEnabled(player))
            EnsureGlyphSlotLocked(player);
    }

    void OnPlayerAfterSpecSlotChanged(Player* player,
                                      uint8 /*newSlot*/) override
    {
        if (IsOocFfEnabled(player))
        {
            RemoveGlyphFromSlot(player, OOC_LOCKED_GLYPH_SLOT);
            EnsureGlyphSlotLocked(player);
        } else
            UnlockGlyphSlot(player);
    }

    bool OnPlayerCanCastItemUseSpell(Player* player, Item* /*item*/,
                                     SpellCastTargets const& /*targets*/,
                                     uint8 /*castCount*/,
                                     uint32 glyphIndex) override
    {
        if (glyphIndex == OOC_LOCKED_GLYPH_SLOT && IsOocFfEnabled(player))
            return false;
        return true;
    }
};

// =====================================================
// Gossip toggle on Keeper Remulos (AllCreatureScript)
// =====================================================
class ooc_ff_remulos_gossip : public AllCreatureScript
{
public:
    ooc_ff_remulos_gossip() : AllCreatureScript("ooc_ff_remulos_gossip") { }

    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (creature->GetEntry() != NPC_KEEPER_REMULOS)
            return false;

        if (!player->GetQuestRewardStatus(QUEST_A_DEEPER_CONNECTION))
            return false;

        // Build the normal gossip menu (DB options + quest icons)
        player->PrepareGossipMenu(creature, creature->GetGossipMenuId(), true);

        // Add the appropriate toggle option from DB (gossip_menu_option 90001)
        bool enabled = IsOocFfEnabled(player);
        if (enabled)
            AddGossipItemFor(player, OOC_GOSSIP_MENU_ID,
                OOC_GOSSIP_OPTION_DISABLE,
                OOC_GOSSIP_SENDER, OOC_GOSSIP_ACTION_TOGGLE);
        else
            AddGossipItemFor(player, OOC_GOSSIP_MENU_ID,
                OOC_GOSSIP_OPTION_ENABLE,
                OOC_GOSSIP_SENDER, OOC_GOSSIP_ACTION_TOGGLE);

        // Override the menuId so SmartAI gossip-select events (which check
        // menuId 10215) do not fire when the player clicks our option.
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(OOC_GOSSIP_MENU_ID);
        SendGossipMenuFor(player, REMULOS_GOSSIP_TEXT, creature);
        return true;
    }

    bool CanCreatureGossipSelect(Player* player, Creature* creature,
                                 uint32 sender, uint32 /*action*/) override
    {
        if (creature->GetEntry() != NPC_KEEPER_REMULOS)
            return false;

        if (sender != OOC_GOSSIP_SENDER)
            return false;

        uint32 guidLow = player->GetGUID().GetCounter();
        bool currentlyEnabled = IsOocFfEnabled(player);
        uint32 responseText;

        if (currentlyEnabled)
        {
            SetOocFfEnabled(player, false);

            if (GetOocFfMask(guidLow) == 0)
                CharacterDatabase.Execute(
                    "DELETE FROM mod_ooc_ff_enabled WHERE guid = {}",
                    guidLow);

            UnlockGlyphSlot(player);
            responseText = NPC_TEXT_OOC_DISABLED;
        }
        else
        {
            uint8 spec = player->GetActiveSpec();

            SetOocFfEnabled(player, true);

            CharacterDatabase.Execute(
                "INSERT IGNORE INTO mod_ooc_ff_enabled (guid, spec) VALUES ({}, {})",
                guidLow, GetOocFfMask(guidLow));

            LockGlyphSlot(player);
            responseText = NPC_TEXT_OOC_ENABLED;
        }

        creature->SendPlaySpellVisual(179);
        creature->SendPlaySpellImpact(player->GetGUID(), 362);

        ClearGossipMenuFor(player);
        SendGossipMenuFor(player, responseText, creature);
        return true;
    }
};

void AddOmenOfClaritySpellScripts()
{
    RegisterSpellScript(spell_ooc_faerie_fire_feral);
    new ooc_ff_player_script();
    new ooc_ff_remulos_gossip();
}
