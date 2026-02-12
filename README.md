# mod-omen-of-clarity

Custom AzerothCore module that empowers Omen of Clarity for druids. When enabled, Faerie Fire (Feral) procs Clearcasting on every hit against non-player targets, provided the druid has Omen of Clarity active.

## Features

- **5-quest herb gathering chain** across all continents to unlock the ability
- **Gossip toggle** on Keeper Remulos to enable/disable at will
- **Glyph slot sacrifice** — enabling locks one major glyph slot as a balance tradeoff
- **Confirmation popup** when enabling via gossip warns about the glyph cost
- **Persistent** across login, level-up, and spec switch

## Quest Chain

1. **The Balance of Life** — Remulos sends you to Rayne (Eastern Plaguelands)
2. **Roots of Kalimdor** — Rayne sends you to a druid trainer in Kalimdor
3. **Life Beyond the Dark Portal** — Druid trainer sends you to Aurine Moonblaze (Netherstorm)
4. **The Titan's Garden** — Aurine sends you to Avatar of Freya (Sholazar Basin)
5. **A Deeper Connection** — Avatar of Freya sends you back to Remulos

Completing quest 5 auto-enables the feature and locks the glyph slot.

## Test Steps

### Prerequisites
- Level 67+ druid
- Clear WDB cache before testing
- Apply SQL to world and characters DBs, rebuild, restart server

### Quest chain walkthrough
```
.go creature 42340          -- Keeper Remulos, pick up quest 1
.additem 13463 2            -- Dreamfoil x2
.additem 8845 2             -- Ghost Mushroom x2
.additem 8846 1             -- Gromsblood x1
.go creature 54186          -- Rayne, turn in quest 1, pick up quest 2

.additem 13468 1            -- Black Lotus x1
.additem 3357 2             -- Liferoot x2
.additem 8846 2             -- Gromsblood x2
.go creature 26661          -- Turak Runetotem (Horde), turn in quest 2, pick up quest 3

.additem 22793 1            -- Mana Thistle x1
.additem 22786 1            -- Dreaming Glory x1
.additem 22791 1            -- Netherbloom x1
.go creature 73594          -- Aurine Moonblaze, turn in quest 3, pick up quest 4

.additem 36908 1            -- Frost Lotus x1
.additem 36905 2            -- Lichbloom x2
.additem 36901 2            -- Goldclover x2
.go creature 110319         -- Avatar of Freya, turn in quest 4, pick up quest 5

.go creature 42340          -- Keeper Remulos, turn in quest 5
```

On quest 5 turn-in: major glyph slot should lock, cast animation plays.

### Gossip toggle
1. Talk to Remulos — click "suppress" — glyph slot unlocks
2. Put a major glyph in the freed slot
3. Talk to Remulos — click "attune" — confirmation popup appears
4. Accept — glyph is removed, slot locks again

### Edge cases
- `.levelup 1` while enabled — slot stays locked
- Log out and back in while enabled — slot stays locked
- Disable, log out and back in — slot is unlocked
- `.additem 40916` (Glyph of Starfire) — try to apply to locked slot, should be blocked
