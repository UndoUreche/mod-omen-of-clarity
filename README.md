## Credits:
**Blinky:** original project and original solution for the implementation
- https://github.com/blinkysc/mod-omen-of-clarity
- https://github.com/blinkysc/mod-classic-glyphs

## Brief:

The mods adds one gameobject to dalaran's inscription hut which allows a druid/paladin player to apply the two wotlk classic glyphs:
- **Glyph of hand of reckoning**
- **Glyph of omen of clarity**

The glyphs are not added as new items, and therefore the mod requires no client side modifications. Applying the glyphs causes the last
glyph slot of the player to be locked on the current spec, and the ability to be modified as per the glyph's description. Disabling the glyph
restores the slot, but leaves it empty.

Only the currently active spec is affected.

## Test Plan:

- Book Tests 
  - Character <80 uses book: generic message
  - Character 80 and unsupported class: generic messages
  - Character 80 and druid: druid message and glyph option
  - Character 80 and paladin: paladin message and glyph option
  - Character 80 druid and glyph learned: druid message and glyph removal option
  - Character 80 paladin and glyph learned: paladin message and glyph removal option

- Glyph tests: druid
  - no glyph cast farie fire (feral) on mob: omen of clarity CAN proc but is not guaranteed
  - glyph cast farie fire (feral) on player pet: omen of clarity CAN proc but is not guaranteed
  - glyph cast farie fire (feral) on player guardian: omen of clarity CAN proc but is not guaranteed
  - glyph cast farie fire (feral) on player: omen of clarity CAN proc but is not guaranteed
  - glyph cast farie fire (feral) on mob: omen of clarity proc is guaranteed
 
- Glyph tests: paladin
  - no glyph cast hand of reckoning on mob: mob is taunted and damage is dealt
  - glyph cast hand of reckoning on player pet: taunt debuff is applied and damage is done (if not aggroed)
  - glyph cast hand of reckoning on player guardian: taunt debuff is applied and damage is done (if not aggroed)
  - glyph cast hand of reckoning on player: immune message is displayed
  - glyph cast hand of reckoning on mob: taunt is not performed, damage is dealt
  - glyph cast hand of reckoning on untauntable mob: taunt is not performed, damage is dealt
