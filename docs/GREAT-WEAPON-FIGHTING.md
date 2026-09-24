# Great Weapon Fighting

[SRD 5.2.1 p. 88](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf#page=88)
requires the Fighting Style feature. For an attack with a Melee weapon held in
two hands, the feat permits treating each damage die showing 1 or 2 as 3. The
weapon must have Two-Handed or Versatile. This replaces die values, not rolls;
it is not the older reroll rule. Feats are nonrepeatable unless stated otherwise.

## Delivered foundation

The internal [damage roller](../src/OpenGold.Rules.Srd5/src/damage_roll.h) supports
an explicit normal or Great Weapon Fighting die rule. Combat delegates its
existing dice calculation to the normal rule, preserving behavior and random
continuation. The helper doubles dice for critical hits, adds the flat modifier
once and floors only the final total at zero. Choosing the replacement consumes
exactly the same random draws. It can be used for each damage component of an
eligible attack when additional attack damage is implemented.

[Damage tests](../tests/damage_tests.cpp) use independent face and seeded-sum
expectations, including two successive critical damage rolls, negative modifiers
and fixed damage. Actual rules 0.6.29 files from `88c2649` preserve a wounded
Fighter with starting Defense, level-four Archery and Soldier Savage Attacker.
The combat files record a critical Greatsword hit before the Savage decision,
after the second roll and after accepting it. Current combat reproduces these
files apart from module identity on later writers, including action expenditure, HP, grants and random state.

## Remaining integration

[#80](https://github.com/stdarg/OpenGold/issues/80) is open. The feat is not yet
selectable or applied in combat. No format or module-version change is introduced
by the foundation. Remaining work includes entitlement/provenance validation,
starting and advancement selection, eligibility from weapon category/properties
and current grip, spell/unarmed/thrown exclusions, reaction attacks, current
save validation and the other class routes. Savage Attacker must show the actual
modified totals and preserve both decisions through internal continuation.

Question 23 is pending: automatically apply the beneficial replacement, or offer
a choice on each eligible hit. The SRD makes use optional; AGENTS.md requires
confirmation of new control behavior. Do not enable either behavior until the
user answers. Existing question 22 approves the starting selector layout, but
does not resolve this combat decision. No combat saving controls are planned.

Verification for the foundation: all 41 native/tool regression checks pass;
main/demo extensions build; Godot combat and Savage Attacker runtime checks and
their native prerequisite pass. No UI layout or localization changes are made.
