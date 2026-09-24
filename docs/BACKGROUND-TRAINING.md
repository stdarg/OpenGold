# Acolyte and Soldier fixed training

[#212](https://github.com/stdarg/OpenGold/issues/212) supplies Acolyte Insight,
Religion and Calligrapher's Supplies, and Soldier Athletics and Intimidation.
The source is [SRD 5.2.1 p. 83](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf#page=83).
Each grant records its background source and acquisition at level one.

All twelve starting classes receive the fixed grants. Rogue Expertise can select
these skills through its existing choice group. Overlapping class/background
proficiency applies once; Expertise doubles proficiency. An applicable proficient
skill/tool combination grants Advantage without adding proficiency again.
Changing backgrounds retains valid choices and clears Expertise that loses its
prerequisite proficiency. Presets use the existing deterministic training choices.

Existing Training and character-sheet controls show names, modifiers and sources
in English and Spanish. No new layout or control behavior is introduced.

Rules 0.6.27 writes PC16 profiles. Training validation distinguishes the original
catalog, PC15's Sage package and PC16's additional background grants. Old profiles
cannot claim future grants; current profiles reject missing or forged grants.
Old combat recipes retain their recorded training. Campaign format 11 reconstructs
owed fixed grants while preserving explicit choices, wounds, resources, equipment
and advancement. Missing selections remain pending; migration grants no items or
new spell choices. FX2 and combat formats 13–15 are unchanged.

The actual [0.6.26 campaign](../tests/fixtures/campaign-v11-backgrounds-before.ogs)
contains a wounded level-three Acolyte Cleric and Soldier Fighter with spent
resources and explicit languages. Tests compare every migrated body byte against
an independently authored addition of exactly five grants and the module identity.
Historical campaign fixtures additionally cover existing equipment and other
backgrounds. [Training tests](../tests/training_tests.cpp) cover all classes,
Expertise, independent check totals, profile validation, migration and level-four
ability improvements. [Godot checks](../tests/training_view_tests.gd) cover the
actual choice groups, background switching, keyboard access and final party sheet.

Acolyte #61 remains open for Magic Initiate and starting equipment/wealth.
Soldier #64 retains its existing Savage Attacker support, but its Gaming Set
selection and starting equipment/wealth remain open. This increment completes
only their fixed skill/tool grants.

Verification: all 40 native/tool regression checks pass; the final focused
training check also verifies valid preceding PC15 Sage profiles. All 16 Godot
runtime checks and seven native fixture prerequisites pass. The actual Training
creation/party flow passes, with both backgrounds rendered and inspected at
1120×800 and 1920×1080 in English and Spanish. Main and demo extensions build;
747 localization messages validate. Scope/architecture review and diff checks pass.
