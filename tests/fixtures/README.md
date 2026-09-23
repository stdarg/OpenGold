# Save migration fixtures

`campaign-v1.ogs` and `campaign-v2.ogs` retain the earlier authored campaign
formats used by the save tests.

`campaign-v6-low-con.ogs` was written by rules module 0.6.2, built from commit
`c59ce07`, before the HP-history correction. Do not regenerate it with the
current writer. It contains six level-four Wizards created through the normal
character/advancement APIs, with Sage bonuses excluding Constitution and a
level-four +2 Constitution increase:

| Member | Species | Starting Constitution | Old maximum/current HP | State |
| --- | --- | --- | --- | --- |
| 1 | Human | 3 | 6 / 6 | Healthy |
| 2 | Human | 3 | 6 / 4 | Wounded |
| 3 | Human | 3 | 6 / 0 | Unconscious, one success/two failures |
| 4 | Human | 3 | 6 / 0 | Dead, one success/three failures |
| 5 | Dwarf | 3 | 10 / 8 | Wounded |
| 6 | Human | 15 | 30 / 28 | Wounded; unaffected by the correction |

All have one remaining slot at each spell level. Migration must produce maxima
9, 9, 9, 9, 13 and 30; current HP 9, 7, 0, 0, 11 and 28; and preserve the resource
strings and dead flag. Subsequent saves must reload without applying the
correction again. The fixture uses no original game assets.


## Combat opportunity-reaction migration

The three `combat-v5-*.save` fixtures were written by rules **0.6.4** from
commit `1c75229`, before removal of facing-triggered reactions. Preserve the
original writer bytes, including trailing spaces. They contain no original assets.

- `combat-v5-facing.save`: a Vanguard at (2,3) has turned left and attacked.
  Its action and Bonus Action are spent, Second Wind is exhausted, HP is 20,
  and 25 feet of movement remain. Guards 3 and 4 await facing-only reactions.
  Migration cancels that queue and changes ticket 4 to 5, preserving every
  other actor, log, RNG, effect and clock field.
- `combat-v5-movement.save`: the same wounded/healed actor attacked to the right,
  then tried to move from (2,2) to (1,2). Guard 3 already declined; guards 4 and
  2 remain queued before the step. Migration must preserve that exact queue.
- `combat-v5-movement-resolved.save`: the old writer's reference continuation
  after guards 4 and 2 take their opportunity attacks. Both miss, movement
  finishes at (1,2), and 25 feet remain. New rules must match all actor, log,
  RNG, clock and resource fields after applying the same commands.
