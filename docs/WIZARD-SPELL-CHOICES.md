# Wizard spell choices — frozen delivery batch

- Authorization: active standing SRD goal. One owner, branch
  `codex/srd-wizard-spell-choices`; no agents/new tasks/issues.
- Existing issues: #97 owns implementation; this batch delivers #37's ordinary
  learning/book/preparation workflow. #97 retains physical books, copying,
  replacement books and its remaining lifecycle acceptance; do not close it for
  this batch. #165 retains the full missing spell catalog.
- Player outcome: separate Wizard knowledge from preparation in real creation,
  advancement, old-save completion and completed-Long-Rest controls through
  level four. Known unprepared spells stay in the book. Replacing preparation
  never learns spells. Level-up keeps existing preparation and can fill the attained capacity; it cannot
  freely replace the existing list. One Wizard cantrip may be replaced per
  completed Long Rest. Level four adds one known cantrip.
- Acceptance: SRD counts 3/3/3/4 cantrips, 6 initial +2 book spells per gained
  Wizard level, 4/5/6/7 prepared spells; acquisition-level spell eligibility,
  independent choices, no duplicates/forged grants or repeated rest entitlements,
  atomic preview/commit, cancel/back, preset choices, old knowledge retained and
  missing knowledge pending. Preserve every supported prior save, spent slots,
  Arcane Recovery, wounds, equipment, RNG and unrelated characters.
- Scope/catalog: all currently implemented Wizard choices (five cantrips;
  Magic Missile, Scorching Ray, Blindness). Preserve full SRD capacities and show
  unfillable selections pending; do not implement new spell effects or claim
  the full Wizard list complete. Existing level/class/source restrictions remain.
- Exclusions: physical book objects, copying costs/time, book replacement/loss,
  Evoker, Ritual Adept, other classes' preparation policies, new spell effects,
  level five+, multiclassing. These remain original requirements; no acceptance
  is removed from #97, #101, #98, #165 or the overall goal.
- Reuse: static SRD spell access/grant validation, generic character history,
  transactional campaign choices/rest eligibility, existing scrollable choice
  controls and shared main/demo dialogs. Core owns transactions, SRD owns counts,
  eligibility, replacement rules and outcomes. No new UI stack or framework.
- Model: requested gpt-6-astra/high retained for independent learning/preparation
  history and completed-rest migration. Actual settings unverified; no switch.
  Escalate out-of-tier decisions or two unsuccessful fixes of the same failure.
- Timing: preflight began at observed 2026-09-25 20:33:54 UTC; frozen by 20:36:35.
  Checkpoint 21:33:54, no later than 22:03:54. Do not reset for approval waits.
- Verification: capture actual 0.6.50 writer fixtures before changing persistence;
  focused access/choice/rest/legacy continuation tests, final affected native and
  Godot checks, main EN/ES and demo EN renders/input at both supported sizes.
- Primary source: SRD 5.2.1 pp.77–78, lines 6497–6500, 6553–6595 in the official
  PDF, verified 2026-09-25. Copying/physical book rules are recorded separately
  in #97 and are explicitly not being claimed as delivered here.

## Layout/control review — pending, do not implement dependent UI

WIZCHOICE-1: Reuse the creator's Spell Choices page with separate scrollable,
counted groups for cantrips, new spellbook entries and preparation. At Wizard
level-up use the existing level/feat/Scholar page followed by a Spell Choices
page in the same window; Back retains edits and final Confirm commits the
whole advancement. Earlier knowledge stays locked; preparation at level-up keeps existing selections and adds spells up to the
attained preparation count. Presets receive pre-generated supported choices;
unsupported catalog choices stay pending instead of shrinking SRD counts.

WIZCHOICE-2: Put a 200-pixel Spellbook button at the right of the existing
Grip/Review Training row below inventory, shortening Grip to 180 pixels and
letting Review Training occupy the middle. At minimum width, x350 label64,
x420 Grip180, x610 Review276, x896 Spellbook200; preserve the row and inventory.
Open a centered 700x700 dialog listing known/prepared spells and scrollable
checkbox groups for pending cantrip/book choices, grouped by acquisition level.
Existing selections stay locked; Apply fills eligible missing knowledge;
Cancel/Escape discards edits. Preparation remains unchanged here. Combat blocks
editing. Existing wounds/resources/equipment and ordinary save controls remain.

WIZCHOICE-3: After a completed Long Rest, present each eligible Wizard's centered
700x700 spell dialog before exploration/encounter continuation. It offers the
prepared-spell checkbox list and optional Replace/With cantrip dropdowns.
Apply commits once; Keep current declines. Choices are limited to known book
spells/current Wizard cantrips, with counts and keyboard access. Canceled or
interrupted rests and Short Rests grant no preparation/replacement entitlement.

## Prior-writer capture — completed before implementation

`opengold_spell_access_tests --capture-wizard-choices` ran against unchanged
module 0.6.50 and refuses another writer version. It produced the four
`campaign-wizard-choices-level*.ogs` fixtures and the combat before/continued
pair. They contain actual attained levels, explicit three-cantrip selections,
level-two Scholar, retained unprepared book entries, wounds, spent spell slots
and spent Arcane Recovery. Campaign 11/15 and PC33/combat continuation are real
writer output, not hand-edited current saves. Existing spell-access tests passed
after capturing. Log: `/tmp/wizard-choices-capture-build.log`.
No gameplay implementation or dependent UI change has begun. Next independent
work can validate/replay these fixtures while control review is pending.
