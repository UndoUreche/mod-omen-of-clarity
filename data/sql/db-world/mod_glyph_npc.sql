-- ============================================================
-- Book
-- ============================================================
DELETE FROM `gameobject_template` WHERE (`entry` = 42325);
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
    (42325, 2, 4677, 'Of Symbols and Sorcery - Part I', '', '', '', 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 'go_classic_glyph_book', 0);

DELETE FROM acore_world.gameobject where guid = 6714437;
INSERT INTO acore_world.gameobject(guid, id, `map`, zoneId, areaId, spawnMask, phaseMask, position_x, position_y, position_z, orientation, rotation0, rotation1, rotation2, rotation3, spawntimesecs, animprogress, state, ScriptName, VerifiedBuild, Comment)
VALUES(6714437, 42325, 571, 0, 0, 1, 1, 5864.489, 698.78595, 644.2837, -1.45735, 0, 0, -0.665882, 0.746057, 900, 100, 1, '', 0, '');

-- ============================================================
-- Book Responses
-- ============================================================
DELETE FROM `npc_text` WHERE `ID` IN (42327, 42328, 42329,42330);
INSERT INTO `npc_text` (`ID`, `text0_0`, `Probability0`, `text1_0`, `Probability1`, `text2_0`, `Probability2`) VALUES
(42327, '<Opening the book grants you clarity, a vision begins to take shape. But it does not last. The text fades away all too quickly, almost as if taunting you.>', 1,
            '<You do not recognise this book. Must be a recent edition.>', 1,
            '<You cannot make out the contents of this book>', 1);

-- druid
INSERT INTO `npc_text` (`ID`, `text0_0`, `Probability0`) VALUES
(42328, '<The letters shift as you open the book...>\n\n [Glyph of Omen of Clarity]\n"Your Omen of Clarity talent has a 100% chance to be triggered by successfully casting Faerie Fire (Feral). Does not trigger on players or player-controlled pets."', 1);

-- paladin
INSERT INTO `npc_text` (`ID`, `text0_0`, `Probability0`) VALUES
(42329, '<The letters shift as you open the book...>\n\n [Glyph of Reckoning]\n"Your Hand of Reckoning spell no longer taunts the target and can deal damage to untauntable targets."', 1);

-- hunter
-- INSERT INTO `npc_text` (`ID`, `text0_0`, `Probability0`) VALUES
-- (42330, '<The letters shift as you open the book...>\n\n [Scroll of Burning Arrow]\n"Modifies your freezing arrow to place an Explosive Trap at the target location that explodes when an enemy approaches, causing  Fire damage and burning all enemies for 20 sec within 10 yards. Trap will exist for 30 sec. Only one trap can be active at a time."', 1);

-- Gossip menu entries for our custom OoC toggle menu (menuId 90001)
DELETE FROM `gossip_menu` WHERE `MenuID` = 90001;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES
(90001, 42328);

-- Gossip option text: enable (OptionID 0) and disable (OptionID 1)
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 90001;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES
(90001, 0, 0, 'Inspect Ancient Sigil.', 0, 1, 1, 0, 0, 0, 0, 'Learning this glyph will seal your third major glyph slot. The glyph within will be destroyed. Do you wish to proceed?', 0),
(90001, 1, 0, 'Purge Sigil.', 0, 1, 1, 0, 0, 0, 0, 'Are you sure?', 0);

-- ============================================================
-- Spell script binding
-- ============================================================
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_ooc_faerie_fire_feral', 'spell_pal_hand_of_reckoning_glyph');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(16857, 'spell_ooc_faerie_fire_feral'),
(62124, 'spell_pal_hand_of_reckoning_glyph');