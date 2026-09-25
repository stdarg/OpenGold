# Medicine stabilization and Tactical Mind — batch proposal

Existing issues: [#32](https://github.com/stdarg/OpenGold/issues/32) and
[#87](https://github.com/stdarg/OpenGold/issues/87). Gameplay implementation has
not started. Q38/Q39 await UI approval; see [decisions](SRD-DECISIONS.md).

## Player outcome and authority

A conscious character can spend a Help Action to stabilize a nearby dying
creature. The check uses the character's actual Wisdom and Medicine training.
A Fighter from level 2 can use Tactical Mind on a failed ability check, rolling
1d10 and spending Second Wind only when the revised check succeeds.

Authority: [SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
p. 18 (Help, DC 10 Wisdom/Medicine, Stable and natural recovery), p. 48
(Tactical Mind, Fighter level 2 and Second Wind), pp. 8–9 (ability checks/training).
The five-foot clear-path aid policy is included explicitly in Q38; it is not
claimed as a range printed in the stabilization paragraph.

## Immutable acceptance

| Route / source | Required behavior and evidence |
| --- | --- |
| All twelve classes' supported levels | Correct untrained/proficient/Expertise Medicine modifier, legal physical targeting, Help Action cost on success and failure, no healing or waking. No automatic success/failure from a natural 20/1 on an ability check. |
| Fighter level 1 | Stabilization works; no Tactical Mind entitlement. |
| Fighter levels 2–4 | Ordinary advancement grants Tactical Mind; it applies to eligible failed ability checks, adds one d10, consumes Second Wind only on success, and never heals. Non-Fighters lack the grant. |
| Turn budgets | Normal Action and eligible Action Surge work; Bonus Action is not consumed. Reaction/off-turn/disabled/pending-choice commands reject without state or RNG changes. |
| Mortality | Success clears death-save counters, stops death saves and starts the existing Stable recovery clock. Damage resumes the existing dying rules; natural recovery keeps Prone. Failed/declined checks change no target state. |
| Resource choices | Decline consumes no d10 or Second Wind. A still-failing boosted check retains Second Wind. Success consumes exactly one use. Short/Long Rest recharge uses the existing service. |
| Continuation | Pending choice, original check, target, action expenditure, resources and RNG survive internal combat checkpoints. Campaign handoff/rest/save preserve final states. No combat-save UI. Capture actual prior-writer fixtures before changing codecs. |
| Player controls | Q38/Q39, game and demo, keyboard/cancel, clear labels and unavailable states; inspect English/Spanish at 1120×800 and 1920×1080. |
| Completion | Inventory all existing ability-check execution paths; do not close #87 while another eligible path bypasses Tactical Mind. Full issue acceptance governs closure. |

## Reuse and boundary

Use the SRD static library's character training/provenance, action budgets,
recovery timeline and the existing Second Wind pool. Introduce a shared check
resolution boundary only with the immediate Medicine/Tactical Mind consumer.
Core owns transactions/persistence; Godot renders legal commands and pending
choices. Do not put SRD thresholds or resource decisions into either layer.

Likely files: `OpenGold.Rules/include/opengold/rules.h`,
`OpenGold.Rules.Srd5/src/srd5.cpp`, character/feature grant validation,
`life_cycle`/`recovery_timeline`, the existing combat views and shared presentation,
plus focused native and Godot tests. Use the current campaign/combat handoff;
no independent health system, UI stack or new GitHub children.

Healer's Kit automation, the Healer feat, general Help advantage, Light attacks,
draw/stow inventory, other Fighting Styles and other class abilities retain their
own existing issues. If an uncovered prerequisite is required for these two
issues, report it before expanding this batch.

## Verification and execution

Run focused checks for independent DC boundaries/modifiers, grant levels,
resource spending, all failure/decline branches, action budgets, Stable timing,
forged/stale requests and exact checkpoint/RNG continuation. Then one relevant
native regression run, game/demo runtime and rendered controls, and actual
campaign/rest/save continuation. Preserve old writer evidence; update coverage,
commit/push and close only fully verified original issues.

Routing: gpt-6-astra/high, retained from the verified preceding batch, for the
shared pending-check decision and save/provenance integration. No model switch,
new task or agents. Escalate/report two failed fixes of the same failure.

Observed batch/preflight start: 2026-09-25 04:41:34 UTC. Review checkpoint
05:41:34; maximum 06:11:34. Do not reset on approval waits or implementation
substeps. Q38/Q39 audio completed successfully; approval widget is visible.


## Verified pre-implementation audit

The `ability_check` path currently derives modifiers only:
`training.cpp::check_modifier` → `CharacterRules::ability_check` →
`RulesModule::ability_check` → `CampaignParty::ability_check`. It preserves
training/provenance, Expertise, tool advantage and equipment penalties. No game
or demo action currently executes a rolled SRD skill check. Combat's other d20
sites are attack rolls, saving throws/death saves and initiative; initiative has
no failed-check DC, so Tactical Mind cannot boost it. Do not apply Tactical Mind
to attacks/saves or reinterpret original ECL random branches as SRD skill checks.
The original surprise adapter in `phlan_session.cpp` uses authored d6 probability
thresholds, not Wisdom/Medicine or a failed SRD ability check.

Medicine will be the first rolled SRD skill-check action. Reuse
`status_effects.cpp::d20` with the proper ability-check modifier and separate
success comparison; do not use attack natural-1/20 rules. Reuse
`life_cycle.cpp::stabilize`: it clears death-save counters, stops their clock and
rolls one d4 for natural Stable recovery without healing. No second stabilization
roll or new recovery timeline is needed. Source/level validation must preserve
old profile access and add only legitimately attained Tactical Mind grants.

Compatibility preparation is complete: the unchanged 0.6.42/PC28 gameplay
libraries wrote a level-two Fighter campaign and two consecutive combat states.
[Fixture provenance and hashes](../tests/fixtures/README.md#tactical-minds-actual-prior-writer)
record the capture. The rebuilt Action Surge suite passes its new prior-writer
continuation test (`/tmp/mind-baseline-tests.log`). No gameplay or UI behavior was
changed. Q38/Q39 remain pending; do not treat this preparation as issue completion.
