# Concentration implementation

[Foundation #207](https://github.com/stdarg/OpenGold/issues/207) implements shared
value-owned transitions in [concentration.h](../src/OpenGold.Rules.Srd5/src/concentration.h).
Authority: [SRD 5.2.1 p. 179](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf#page=179).

Each owner has at most one active application, identified by encounter scope,
caster and application ID. Beginning another effect returns the old source for
cleanup, including recasts. Validation runs before replacement. Integration must
call this at the start of a new concentration spell's casting, not after its hit
or save resolves; invalid commands must be rejected before starting the cast.

Expiry, voluntary release and incapacitation return the ended source. Damage
uses the existing Constitution saving throw machinery, including advantage and
disadvantage. DC is half damage rounded down, bounded to 10–30. Zero damage and
inactive concentration do not roll; incapacitation/death ends without a save.
The caller supplies damage after defenses and before Temporary HP absorption,
and supplies the resulting incapacitated/dead state. A natural 1 or 20 does not
supersede the saving throw total. No actor ownership or combat pointers are held.

[Tests](../tests/concentration_tests.cpp) cover source replacement, invalid-input
atomicity, separate owners/scopes, Silence's 600,000 ms duration, exact expiry
and time partitioning, DC boundaries, save outcomes, modifier cancellation, RNG
draw counts and cleanup identity. Concentration, status-effect and Ray of Frost
focused tests pass. This isolated helper does not change existing gameplay,
save formats or module version.

## Integration still required

- [#208](https://github.com/stdarg/OpenGold/issues/208): actual Silence access,
  persistent sphere geometry, sound/Verbal restrictions, Deafened and Thunder
  immunity, combat damage/concentration hooks, player controls and checkpoints.
- [#209](https://github.com/stdarg/OpenGold/issues/209): campaign location/time,
  ritual casting, damage and recovery, camping/inn persistence and sound-dependent
  interactions. Source routes remain Cleric, Bard (#126) and Ranger (#145).
- [#39](https://github.com/stdarg/OpenGold/issues/39): mundane gagging also needs
  explicit source and removal behavior; magical silence does not deliver it.

The foundation is not playable concentration or Silence. Parents #38/#39/#43,
the spell inventory entry and campaign tracker #174 remain open until their
actual acceptance criteria pass. Silence's new area/control layout needs user
review before implementation; no new UI approval has been inferred.
