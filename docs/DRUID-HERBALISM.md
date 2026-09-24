# Druid Herbalism Kit proficiency

[#216](https://github.com/stdarg/OpenGold/issues/216) implements the fixed tool
proficiency from Core Druid Traits, SRD 5.2.1 p. 41. Equipment p. 94 defines the
Herbalism Kit and its Intelligence-based checks.
[Official SRD](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

Every normally created Druid and generated Druid preset receives
`tool:herbalism_kit` from `class:druid` at level one. The existing fixed Training
summary and completed sheet show the grant and its translated source. There is
no new choice or control. This grants proficiency, not inventory ownership.

The generic ability-check query adds proficiency once and applies existing
skill/tool Advantage when both apply. Other classes do not receive this fixed
grant, and Herbalism Kit is excluded from Monk's artisan/instrument choices.
Missing, duplicate and forged grants are rejected.

## Persistence

Rules 0.6.33 / PC22 introduces the fixed entitlement policy. Earlier combat
recipes retain their original policies and exact continuation. Campaign replay
adds the owed fixed grant after validating the historical ledger without it.
Only the new grant and module identity/checksum change; recorded choices,
including pending choices, wounds and resources remain intact. Repeated
save/load does not duplicate proficiency. New grants reject under old identities.

The actual 0.6.32 writer at `c0457f8` generated four Druid campaign/combat fixtures
before production changes, covering all current backgrounds, Nature/Medicine,
Elvish/Dwarvish, two missing HP and recorded resources. The frozen PC21 recipes
remain unedited. The independently authored campaign oracle adds only the
specified fixed grant; it does not call current grant generation or save decode.

## Verification

Focused checks cover all twelve classes/four backgrounds, exact provenance,
proficiency and Advantage, rejected grants/versions, current and historical
combat, exact campaign migration, repeated loading and preset generation.
The existing all-class fixture also verifies previously pending Druid skills
remain pending after the fixed grant is added. Godot checks cover normal Druid
creation, fixed Training text, completed sheet and translated source labels.
All 41 native/tool checks and all 16 Godot runtime checks (plus seven native
prerequisites) pass. Main/demo extensions build; 806 localized messages validate.
The graphical normal-creation test passes, including the completed Druid sheet.
English/Spanish fixed-grant text was inspected at 1120×800 and 1920×1080.
Artifacts: `/tmp/opengold-druid-renders`; logs: `/tmp/opengold-druid-regression.log`
and `/tmp/opengold-druid-render.log`.

Druidic, Spellcasting, Primal Order, equipment ownership, crafting/Utilize actions,
and multiclass entry remain separate. This does not complete #151 or #52.
