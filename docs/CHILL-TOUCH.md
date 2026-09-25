# Chill Touch — active implementation batch

Existing inventory owner: #165, Chill Touch row in SPELL-INVENTORY.md; healing
restriction also contributes to #35. No new issue is authorized or needed under
the batching workflow. This is one complex spell, not completion of the spell
inventory or its still-missing species/feat access routes.

Authority: SRD 5.2.1 p. 115: Sorcerer/Warlock/Wizard Necromancy cantrip, Action,
Touch, V/S; melee spell attack, 1d10 Necrotic at character levels 1–4, prevention
of Hit Point recovery until the end of the caster's next turn. Do not copy the
2014 ranged attack, d8 damage, or special undead rider.

## Fixed player outcome

Add the spell to the existing Wizard/Sorcerer/Warlock Spell Choices and Spell/Cast
path, using their real grants, counts and Intelligence/Charisma. Old selections
stay unchanged; no unsolicited spell assignment. Preserve blinded/prone/cover,
critical hits, physical spell components, source/target legality, action budget,
slots, failed-command atomicity and deterministic continuation.

Apply sourced, nonstacking healing prevention with exact end-of-next-caster-turn
expiry, including dead/skipped caster turns and campaign handoff. All actual HP
recovery paths must respect the effect; Temporary HP is not HP recovery. Preserve
wounds/resources and older saves. Test simultaneous effect/recovery boundaries,
multiple casters, immunity/zero resolved damage, natural death-save recovery,
spell/Second Wind healing, campaign time, rest and script healing.

The existing shared controls are approved by Q18 (fixed additions need no new
layout question), with class access patterns Q12/Q26/Q27. No new dialog or
control. Render existing choices/status in English and Spanish at both sizes. The shared
creator is used by game/demo; combat Spell/Cast is main-game only, as explicitly
recorded in CANTRIP-CONTROLS.md. Standalone demo combat controls remain unchanged.
Creature combat casting uses the existing targeting system. Object targeting,
Magic Initiate, Tome, High Elf, Chthonic Tiefling and higher class progression
retain their recorded owners; the inventory stays explicitly incomplete for
those routes. Do not silently declare the entire spell or #165 complete.

## Execution

Routing: retained gpt-6-astra/high for healing/mortality/effect-timeline and
save migration interactions. No delegation or model switch. Current prior-writer
fixtures include actual 0.6.42/PC28 campaign/combat captures and held-item FX4
fixtures. Capture any additional boundary needed before modifying the writer.

Observed start 2026-09-25 04:56:05 UTC; checkpoint 05:56:05, maximum 06:26:05.
The Medicine/Tactical Mind batch is deferred pending Q38/Q39; its original clock
04:41:34 is retained, not reset. Resume it after its approval, preserving the
completed audit and prior-writer fixtures. Initial implementation is preserved on the unfinished
`codex/srd-chill-touch` branch; focused native Chill Touch and Sorcerer tests passed. An old test
that searched a hard-coded current module identity was updated for 0.6.43.

Q40 is pending (audio and widget delivered): if Stable natural recovery becomes
due during healing prevention, defer its already-earned 1 HP until the block
expires, without another d4-hour roll. Do not implement that interpretation
until answered. Natural recovery integration and its boundary tests remain
incomplete; do not deliver this spell as finished or close an issue yet.


## WIP verification (not completion)

Implementation stays in `opengold_rules_srd5` STATIC. Core and public RulesModule
interfaces are unchanged. Main-game Godot adds only the spell ID/label and routes
it through existing targeting/audio/selection code. Released choices are retained;
new profiles use PC29 only for the new mask. FX5 stores prevention and existing
posture flags; 0.6.42 and all older supported modules remain accepted.

Focused tests cover all three sources and casting abilities, starting counts,
levels 1–4 damage, critical hits, Necrotic affinities including immunity, Magic
cost, range/hands/armor, stale commands, independent overlapping sources, normal
and skipped/dead caster turns, actual spell/Second Wind/temple/Hit Die/script HP
recovery, Temporary HP, death-save natural 20, current saves, actual PC/NPC combat
handoff, campaign expiry and ordinary camping. Natural Stable recovery suppression
and its timeline/serialization policy remain deliberately unimplemented pending
Q40; passing existing tests does not make this spell complete.

- Eight focused native/Godot checks passed (`/tmp/chill-integration-tests.log`).
- Native regression: 45/46 initially passed; the sole stale component-count
  assertion was updated from eleven to twelve and its rebuilt test passed.
  Thus all 46 current native/tool checks have passing results. Logs
  `/tmp/chill-native-{build,tests}.log`, `/tmp/chill-component-{build,test}.log`.
- Added dead/unconscious-caster and real PC/NPC handoff/rest cases then rebuilt
  and passed the feature test (`/tmp/chill-final-focused{,-build}.log`).
- Main-game combat test passed for all three classes in EN/ES, keyboard casting,
  ally targeting, internal load continuation and disabled player save buttons.
  Rendered all three classes at 1120×800 and 1920×1080; inspected Spanish minimum
  and English large views. `/tmp/chill-captures`, `/tmp/chill-render.log`.
- Main-game creation selection/Back/keyboard/final-sheet check passed in EN/ES;
  rendered both sizes and inspected Spanish minimum choices. A copied test's
  incorrect "Chill Grasp" text was corrected to "Chill Touch" before its pass.
  `/tmp/chill-creator.log`, `/tmp/chill-creator-captures`.
- Full Godot regression: 23/23 runtime checks passed (36 including native
  prerequisites), `/tmp/chill-godot-regression.log`.
- Rebuilt standalone demo, then passed shared creator keyboard/Back/final-sheet
  checks and rendered/inspected both sizes. This existing demo translates via
  `gs(source)` and has no configured translations, so its check is English-only;
  the main-game EN/ES requirement remains fully tested. The initial attempt to
  impose Spanish expectations on the standalone demo was corrected in the test,
  without changing demo localization or UI. `/tmp/chill-demo-{build,creator}.log`,
  `/tmp/chill-demo-creator-captures`.
- Localization: 876 main-game messages complete and valid. Diff check passes.
- No shipped fixture bytes were rewritten. The previous Mind baseline writer
  remains gated to its actual 0.6.42 binary, rather than capturing new bytes under
  an old label. No user data changes are retained by combat fixture checks.

This WIP branch is separate from main because Q40 prevents a complete verified
spell delivery. No GitHub issue is closed, and no finished coverage row is added.
The original scope and timing remain fixed; finish this increment after its
required decision instead of silently adding another work batch.

Observed independent verification complete 2026-09-25 05:23:42 UTC (27m37s since
batch start, still before the unchanged 05:56:05 checkpoint). No live build/test
process remains. Q40-dependent implementation and verification remain outstanding.
No measured token/cost saving is claimed. No two unsuccessful fixes of one failure.
