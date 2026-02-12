-- ============================================================
-- mod-omen-of-clarity: "The Balance of Life" quest chain
-- ============================================================
-- Chain: Remulos → Rayne (EK) → Turak/Sheldras (Kalimdor) → Aurine (Outland) → Avatar of Freya (Northrend) → Remulos
--
-- NPC entries:
--   11832  Keeper Remulos (Moonglade)
--   16135  Rayne (Eastern Kingdoms, Cenarion Circle)
--    3033  Turak Runetotem (Thunder Bluff, Horde druid trainer)
--    5504  Sheldras Moontree (Stormwind, Alliance druid trainer)
--   20871  Aurine Moonblaze (Netherstorm, Cenarion Expedition)
--   27801  Avatar of Freya (Sholazar Basin)

-- Clean up old single-quest data from previous version
DELETE FROM `creature_template` WHERE `entry` IN (100001, 100002, 100003);
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (100001, 100002, 100003);
DELETE FROM `item_template` WHERE `entry` IN (100001, 100002, 100003);
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_ooc_cenarion_offering';
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (100001, 100002, 100003, 100004, 100005);

-- ============================================================
-- Quest templates
-- ============================================================
DELETE FROM `quest_template` WHERE `ID` IN (100001, 100002, 100003, 100004, 100005);

-- Quest 1: Remulos sends you to Rayne in Eastern Kingdoms (bring EK herbs)
INSERT INTO `quest_template` (
    `ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`,
    `QuestInfoID`, `Flags`,
    `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`,
    `RequiredItemId1`, `RequiredItemCount1`,
    `RequiredItemId2`, `RequiredItemCount2`,
    `RequiredItemId3`, `RequiredItemCount3`,
    `RewardXPDifficulty`, `RewardNextQuest`, `AllowableRaces`
) VALUES (
    100001, 2, -1, 67, -1, 0, 0,
    'The Balance of Life',
    'Bring 2 Dreamfoil, 2 Ghost Mushroom, and 1 Gromsblood to Rayne in Eastern Plaguelands.',
    'The balance of nature is in peril, $N. I have felt a disturbance rippling through the Emerald Dream - the old roots of Azeroth cry out for restoration.$B$BAn old friend of the Circle, Rayne, has been tending to the wounded lands of the Eastern Kingdoms. Seek her out and bring her these herbs as an offering. She will know what to do.$B$BRemember: to preserve life, one must sometimes sacrifice it.',
    'Bring the herbs to Rayne.',
    13463, 2,  -- Dreamfoil x2
    8845, 2,   -- Ghost Mushroom x2
    8846, 1,   -- Gromsblood x1
    5, 100002, 0
);

-- Quest 2: Rayne sends you to Kalimdor (bring Kalimdor herbs)
INSERT INTO `quest_template` (
    `ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`,
    `QuestInfoID`, `Flags`,
    `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`,
    `RequiredItemId1`, `RequiredItemCount1`,
    `RequiredItemId2`, `RequiredItemCount2`,
    `RequiredItemId3`, `RequiredItemCount3`,
    `RewardXPDifficulty`, `RewardNextQuest`, `AllowableRaces`
) VALUES (
    100002, 2, -1, 67, -1, 0, 0,
    'Roots of Kalimdor',
    'Bring 1 Black Lotus, 2 Liferoot, and 2 Gromsblood to a druid trainer in Kalimdor.',
    'The herbs you brought have mended what I could not alone, $N. The land here breathes easier now.$B$BBut the imbalance runs deeper. In Kalimdor, the ancient groves still remember the sundering. Seek out a druid trainer there - Turak Runetotem in Thunder Bluff if you walk with the Horde, or Sheldras Moontree in Stormwind Park if you stand with the Alliance. Bring them the herbs of their homeland.',
    'Bring the herbs to a druid trainer in Kalimdor.',
    13468, 1,  -- Black Lotus x1
    3357, 2,   -- Liferoot x2
    8846, 2,   -- Gromsblood x2
    5, 100003, 0
);

-- Quest 3: Turak/Sheldras sends you to Outland (bring Outland herbs)
INSERT INTO `quest_template` (
    `ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`,
    `QuestInfoID`, `Flags`,
    `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`,
    `RequiredItemId1`, `RequiredItemCount1`,
    `RequiredItemId2`, `RequiredItemCount2`,
    `RequiredItemId3`, `RequiredItemCount3`,
    `RewardXPDifficulty`, `RewardNextQuest`, `AllowableRaces`
) VALUES (
    100003, 2, -1, 67, -1, 0, 0,
    'Life Beyond the Dark Portal',
    'Bring 1 Mana Thistle, 1 Dreaming Glory, and 1 Netherbloom to Aurine Moonblaze in Netherstorm.',
    'You carry the scent of wild places, $N. The work you do honors the old ways.$B$BBut the balance extends beyond this world. In Outland, the Cenarion Expedition fights to restore life to a shattered land. Aurine Moonblaze tends the wilds of Netherstorm - she will know how to use the strange herbs that grow in that broken place. Bring her what she needs.',
    'Bring the herbs to Aurine Moonblaze.',
    22793, 1,  -- Mana Thistle x1
    22786, 1,  -- Dreaming Glory x1
    22791, 1,  -- Netherbloom x1
    5, 100004, 0
);

-- Quest 4: Aurine sends you to Northrend (bring Northrend herbs)
INSERT INTO `quest_template` (
    `ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`,
    `QuestInfoID`, `Flags`,
    `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`,
    `RequiredItemId1`, `RequiredItemCount1`,
    `RequiredItemId2`, `RequiredItemCount2`,
    `RequiredItemId3`, `RequiredItemCount3`,
    `RewardXPDifficulty`, `RewardNextQuest`, `AllowableRaces`
) VALUES (
    100004, 2, -1, 67, -1, 0, 0,
    'The Titan''s Garden',
    'Bring 1 Frost Lotus, 2 Lichbloom, and 2 Goldclover to the Avatar of Freya in Sholazar Basin.',
    'The herbs have taken root beautifully, $N. Even here in the Nether, life finds a way.$B$BBut there is one more place that needs your touch. In Northrend, the Avatar of Freya watches over the last garden of the titans in Sholazar Basin. The cold of that continent breeds hardy herbs unlike any other. Seek her out and bring the flowers of Northrend to her ancient grove.',
    'Bring the herbs to the Avatar of Freya.',
    36908, 1,  -- Frost Lotus x1
    36905, 2,  -- Lichbloom x2
    36901, 2,  -- Goldclover x2
    5, 100005, 0
);

-- Quest 5: Avatar of Freya sends you back to Remulos
INSERT INTO `quest_template` (
    `ID`, `QuestType`, `QuestLevel`, `MinLevel`, `QuestSortID`,
    `QuestInfoID`, `Flags`,
    `LogTitle`, `LogDescription`, `QuestDescription`, `QuestCompletionLog`,
    `RewardXPDifficulty`, `AllowableRaces`
) VALUES (
    100005, 2, -1, 67, -1, 0, 0,
    'A Deeper Connection',
    'Return to Keeper Remulos in Moonglade.',
    'I can feel it, mortal. The threads of life you have woven across these worlds resonate with an ancient harmony. You have walked where few druids dare - from the plagued lands to the shattered world, and now to the titan''s garden.$B$BReturn to Keeper Remulos. He will sense what you have become. The wild has marked you, and its clarity is yours to command.',
    'Return to Keeper Remulos in Moonglade.',
    8, 0
);

-- ============================================================
-- Quest chain linking and class restriction (quest_template_addon)
-- ============================================================
DELETE FROM `quest_template_addon` WHERE `ID` IN (100001, 100002, 100003, 100004, 100005);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`, `NextQuestID`) VALUES
(100001, 1024, 0, 0),      -- Druid only (1024 = CLASS_DRUID), first quest
(100002, 1024, 100001, 0),  -- Requires quest 100001
(100003, 1024, 100002, 0),  -- Requires quest 100002
(100004, 1024, 100003, 0),  -- Requires quest 100003
(100005, 1024, 100004, 0);  -- Requires quest 100004

-- ============================================================
-- Quest givers and enders
-- ============================================================
-- Quest 1: Remulos gives, Rayne ends
DELETE FROM `creature_queststarter` WHERE `quest` IN (100001, 100002, 100003, 100004, 100005);
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(11832, 100001),  -- Keeper Remulos gives quest 1
(16135, 100002),  -- Rayne gives quest 2
(3033,  100003),  -- Turak Runetotem gives quest 3 (Horde)
(5504,  100003),  -- Sheldras Moontree gives quest 3 (Alliance)
(20871, 100004),  -- Aurine Moonblaze gives quest 4
(27801, 100005);  -- Avatar of Freya gives quest 5

DELETE FROM `creature_questender` WHERE `quest` IN (100001, 100002, 100003, 100004, 100005);
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(16135, 100001),  -- Rayne ends quest 1
(3033,  100002),  -- Turak Runetotem ends quest 2 (Horde)
(5504,  100002),  -- Sheldras Moontree ends quest 2 (Alliance)
(20871, 100003),  -- Aurine Moonblaze ends quest 3
(27801, 100004),  -- Avatar of Freya ends quest 4
(11832, 100005);  -- Keeper Remulos ends quest 5

-- ============================================================
-- Quest POI (map markers for turn-in locations)
-- ============================================================
DELETE FROM `quest_poi` WHERE `QuestID` IN (100001, 100002, 100003, 100004, 100005);
DELETE FROM `quest_poi_points` WHERE `QuestID` IN (100001, 100002, 100003, 100004, 100005);

INSERT INTO `quest_poi` (`QuestID`, `id`, `ObjectiveIndex`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`) VALUES
(100001, 0, -1, 0,   23,  0, 0, 1),  -- Turn in at Rayne (Eastern Plaguelands)
(100002, 0, -1, 1,   9,   0, 0, 1),  -- Turn in at Turak Runetotem (Thunder Bluff, Horde)
(100002, 1, -1, 0,   301, 0, 0, 1),  -- Turn in at Sheldras Moontree (Stormwind, Alliance)
(100003, 0, -1, 530, 479, 0, 0, 1),  -- Turn in at Aurine Moonblaze (Netherstorm)
(100004, 0, -1, 571, 493, 0, 0, 1),  -- Turn in at Avatar of Freya (Sholazar Basin)
(100005, 0, -1, 1,   241, 0, 0, 1);  -- Turn in at Keeper Remulos (Moonglade)

INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`) VALUES
(100001, 0, 0, 2277,  -5329),  -- Rayne
(100002, 0, 0, -1039, -282),   -- Turak Runetotem
(100002, 1, 0, -8776, 1100),   -- Sheldras Moontree
(100003, 0, 0, 4246,  3124),   -- Aurine Moonblaze
(100004, 0, 0, 5876,  4117),   -- Avatar of Freya
(100005, 0, 0, 7848,  -2216);  -- Keeper Remulos

-- ============================================================
-- Quest offer reward text (shown before clicking "Complete Quest")
-- ============================================================
DELETE FROM `quest_offer_reward` WHERE `ID` = 100005;
INSERT INTO `quest_offer_reward` (`ID`, `Emote1`, `RewardText`) VALUES
(100005, 66, 'You have walked the breadth of this world and beyond, tending to the roots of life itself. I can feel the harmony resonating within you, $N.$B$BThe wild has chosen you. Its clarity will flow through your Faerie Fire, striking with purpose renewed.$B$BBut know this: such power demands sacrifice. One of your major glyph slots will be sealed while this gift remains active. You may return to me if you ever wish to release or reclaim it.');

-- ============================================================
-- Spell script binding (Faerie Fire proc only)
-- ============================================================
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_ooc_faerie_fire_feral';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(16857, 'spell_ooc_faerie_fire_feral');

-- ============================================================
-- Gossip toggle for empowered Omen of Clarity (Keeper Remulos)
-- ============================================================
-- NPC text shown after toggling
DELETE FROM `npc_text` WHERE `ID` IN (90001, 90002);
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES
(90001, 'I can sense it, $N. Your bond with the wild pulses strongly once more. The clarity of nature flows through your Faerie Fire.$B$BShould you ever wish to quiet this gift, return to me.'),
(90002, 'The gift sleeps within you still, $N. When you are ready to rekindle your bond with the wild, seek me out again.$B$BThe clarity of nature is patient. It will wait.');

-- Gossip menu entries for our custom OoC toggle menu (menuId 90001)
DELETE FROM `gossip_menu` WHERE `MenuID` = 90001;
INSERT INTO `gossip_menu` (`MenuID`, `TextID`) VALUES
(90001, 90001);

-- Gossip option text: enable (OptionID 0) and disable (OptionID 1)
DELETE FROM `gossip_menu_option` WHERE `MenuID` = 90001;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`) VALUES
(90001, 0, 0, 'I wish to attune my connection to the wild.', 0, 1, 1, 0, 0, 0, 0, 'Awakening this power will seal your third major glyph slot. The glyph within will be destroyed. Do you wish to proceed?', 0),
(90001, 1, 0, 'I wish to suppress my connection to the wild.', 0, 1, 1, 0, 0, 0, 0, '', 0);
