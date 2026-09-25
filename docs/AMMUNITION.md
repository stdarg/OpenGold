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
- Existing weapon metadata combines firearm/sling bullets; compatible inventory
  selection must distinguish the two ordinary types.
- Original item types 28 (Quarrel) and 73 (Arrows) are recognized by the format
  catalog but not converted to ordinary ammunition by campaign inventory.

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
