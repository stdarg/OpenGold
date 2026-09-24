# Soldier Gaming Set proficiency

[#217](https://github.com/stdarg/OpenGold/issues/217) implements the Soldier
background's one Gaming Set proficiency choice (p. 83). The SRD 5.2.1 Equipment section
(p. 94) lists four separately proficient variants: dice, dragonchess, playing
cards and three-dragon ante. Gaming Set checks ordinarily use Wisdom.
[Official SRD](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf).

## Player behavior

The existing Training checkbox groups require one selection for Soldiers of
all twelve classes. Keyboard focus, counts/limits and Back preservation reuse
the approved controls. Changing class preserves the background choice; changing
to a different background removes the no-longer-owed entitlement. Presets
receive generated choices through the existing rules-driven generator.

The sheet shows `tool:<variant>` from `background:soldier:gaming_set`, level one.
Checks add proficiency once and existing skill/tool Advantage when applicable.
Tools now have one explicit category (other, artisan, instrument or gaming), so
Monk's and Bard's catalogs retain their boundaries. Proficiency does not award
an inventory item or implement a Gaming Set Utilize action.

Rules 0.6.34 / PC23 validates the new entitlement policy and rejects incompatible
sources/versions. Campaign 11 and combat 13–15 have no schema changes. Old missing
choices remain pending; core completion adds the new choice without replacing
old selections or changing wounds, resources, advancement or feat grants. The
separately pending Review Training UI remains #189.

## Migration and verification

The actual 0.6.33 libraries at `ba9673c` generated campaign/combat fixtures before
production changes. All twelve classes have Soldier, complete prior training,
Savage Attacker, wounds and recorded resource state. Fighter, Cleric and Wizard
have level-four histories. Campaign PCs are in reserve to fit the six-active-PC
limit; combat includes all twelve. The generator requires the prior version,
and fixture bytes remain unedited.

Tests cover all 48 class/variant combinations, exact catalogs/counts/provenance,
bonuses, rejection atomicity, class/background changes, presets, current saves,
exact old-save migration and safe completion. Existing old Bard/Monk/Druid tests
also retain pending Soldier choices until explicitly completed. Godot exercises
keyboard selection, limit, Back, class-change preservation and completed-sheet
sources through ordinary creation.

All 41 native/tool checks pass across the regression run and targeted reruns.
Five older training-completion tests needed explicit Gaming Set selections;
those five passed after updating their authored choices. All 16 Godot runtime
checks (plus seven native prerequisites) and the graphical Training flow pass.
Main/demo extensions build; 811 localized messages validate. English/Spanish
Gaming Set controls were inspected at 1120×800 and 1920×1080. Evidence:
`/tmp/opengold-soldier-regression.log`, `/tmp/opengold-soldier-recheck.log`,
`/tmp/opengold-soldier-render.log`, `/tmp/opengold-soldier-renders`.

Starting equipment and wealth remain #64. This proficiency increment does not
complete the full Soldier background package.
