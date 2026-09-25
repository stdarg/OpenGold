# Ammunition — #57 delivery packet

## Frozen batch

- Authorization: standing SRD goal; one active batch, #57 only. Branch
  `codex/srd-ammunition`. No new issues, tasks or agents.
- Outcome: ranged ammunition weapons spend matching inventory ammunition;
  eligible post-fight recovery returns the permitted amount; campaign saves and
  supported combat checkpoints preserve quantities and spent resources.
- Acceptance: all nine catalog ammunition weapons, five ordinary ammunition
  types (firearm and sling bullets are distinct), one piece per attack hit or
  miss, free hand for loading a one-handed weapon, empty/wrong-ammunition
  rejection without mutation, once-only recovery and time advancement.
  Preserve stack identities and imported provenance. Apply through shared
  rules to all twelve classes at supported levels and converted NPC equipment.
- Include ordinary original ammunition conversion through existing inventory
  acquisition routes. Do not silently convert enchanted/unsupported items.
- Exclude Loading/Extra Attack (#56), mastery, magic ammunition effects, new
  shopping/crafting systems and unrelated fighting styles. Existing released
  saves remain supported; no invented ammunition in old player inventories.
- Reuse #58 physical inventory transactions, existing Ranged controls, Q37
  equipment return policy and existing campaign clock/effect advancement.
  Additional behavior is pending AMMO-1/AMMO-2 in the decision register.
- Rules outcomes belong to `opengold_rules_srd5`; Core validates/applies generic
  inventory transactions and time; Godot displays rules-owned options/results.
- Requested assignment: `gpt-6-astra` / `high`, retained for persistence and
  recovery interactions. Actual configuration not independently verified; no
  model/runtime change. Owner: current coordinator. Failed fixes: zero.
  Escalate unresolved architecture/policy or two unsuccessful same-failure fixes.
- Verification: actual 0.6.47 writer fixtures before runtime edits; focused
  ammunition, inventory and save tests; affected native regression; game/demo
  controls and EN/ES layouts at 1120x800 and 1920x1080 after UI approval.
- Timing: investigation began 2026-09-25 17:58:09 UTC; checkpoint 18:58:09,
  latest 19:28:09. Preparation is not a delivered player requirement.

## Source and decisions

[SRD 5.2.1](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
printed page 89: matching ammunition, one piece per ranged attack, drawing
included, free loading hand for one-handed weapons, optional one-minute recovery
of half ammunition used (rounded down). Page 96 distinguishes arrows, bolts,
firearm bullets, sling bullets and needles. Page 255 supplies monsters with the
ammunition their stat blocks need; this does not supply player characters.

AMMO-1 proposes automatic first-compatible ordinary stack selection and Ranged
availability/count feedback. AMMO-2 proposes a post-victory recovery dialog,
rounding by character/type across stacks, and collection by able companions.
These proposals are not approved by this packet. No dependent UI/policy changes
until the user answers. Scope questions do not pause unrelated authorized work
inside this batch or the standing goal.

## Findings to resolve within this batch

- Current physical inventory conserves every unit and rejects quantity loss.
  Expenditure needs explicit generic accounting, not SRD arithmetic in Core.
- Resolved in the inventory substep: distinct firearm/sling bullet metadata and
  ordinary original item conversions for type 28 (Quarrel) and 73 (Arrows).

## Delivery

No runtime delivery or issue closure yet. Compatibility preparation completed
2026-09-25 around 18:09 UTC. Actual 0.6.47 campaign/combat captures and a focused
continuation test pass; [fixture provenance and hashes](../tests/fixtures/README.md#ammunition-baseline-actual-0647-writer).
Commands: `cmake -S . -B build/mac-check` (new test target),
`cmake --build build/mac-check --target opengold_ammunition_tests -j6`,
`build/mac-check/opengold_ammunition_tests --capture-prior-writer`, and
`ctest --test-dir build/mac-check --output-on-failure -R '^opengold_ammunition_tests$'`.
Build log: `/tmp/ammunition-baseline-build.log`. Runtime remains `5d7813d`.

The first campaign fixture experiment used unrecognized authored arrow keys;
the old reader rejected them. Corrected the capture to acquire original arrows
and quarrels through the existing purchase/provenance path, then regenerated
with the same old writer and verified loading before freezing. No production
codec or compatibility requirement was weakened. One fixture setup correction;
no repeated failed production fix. AMMO-1/AMMO-2 remain pending.

## Inventory substep — pending full #57 delivery

Module 0.6.48 recognizes five ordinary ammunition keys as carried supplies.
Core asks the generic equipment interface whether an item is carried rather
than equipped; ammunition definitions and weapon compatibility remain SRD-owned.
This enables authored stacks to survive campaign validation and prevents an
Equip request from displacing a weapon or mutating state.

Ordinary original arrows/quarrels now convert to `arrow`/`bolt` on acquisition.
The existing provenance-checked migration also updates old unsupported keys,
preserving IDs, quantities, original records and equipped weapons. Magic, cursed
and effect-bearing original ammunition remains unsupported, with its provenance
unchanged. Firearm and sling bullets have separate keys and catalog identities.
Module 0.6.47 campaign and combat continuation remains accepted; the actual
prior-writer fixtures are unchanged and the capture command rejects 0.6.48.

Focused checks passed: `opengold_ammunition_tests`,
`opengold_weapon_catalog_tests`, `opengold_thrown_weapon_tests`.
The ammunition test covers all twelve classes, five authored ammunition types,
canonical campaign reload, rejected Equip atomicity, original conversions,
special-item preservation and actual old campaign/combat continuation.
Logs: `/tmp/ammunition-inventory-build.log` and
`/tmp/ammunition-inventory-focused.log`.

All 48 affected native executable targets rebuilt successfully; all 50
native/tool tests passed (14.51 seconds) on the resulting tree on 2026-09-25
at approximately 18:18 UTC. Commands: native target list from CTest's JSON
inventory, `cmake --build build/mac-check --target <native targets> -j6`, then
`ctest --test-dir build/mac-check --output-on-failure -E '^opengold_godot_' -j6`.
Logs: `/tmp/ammunition-native-build.log`, `/tmp/ammunition-inventory-native.log`.
Tested tree is this inventory substep commit on `codex/srd-ammunition`, parent
`1fe531d`; no runtime inputs changed after verification. Existing frozen
ammunition save hashes match the recorded capture. No UI code was changed or
new controls implemented; Godot control/render verification belongs to the
remaining approved UI implementation.

Firing expenditure, free-hand enforcement, recovery and control feedback remain
unfinished; AMMO-1/AMMO-2 are still pending. No issue closure or complete gameplay
claim for this substep. Next action: resolve the already-visible decisions and
continue the same batch, retaining the original 18:58:09 checkpoint. Do not
restart the clock or repeat this verified inventory work.
