# SRD 5.2.1 demonstration content

`combat.rules` is a small, offline, curated combat pack, revision
`srd-5.2.1-demo.1`. The runtime validates it directly; it makes no HTTP requests.
Open5E provides reference data, while OpenGold implements executable behavior.

`open5e-bandit.json` is the complete Open5E response fetched on 2026-09-06 from:

https://api.open5e.com/v2/creatures/?document__key=srd-2024&name__iexact=Bandit

The document filter deliberately excludes other editions and third-party
sources. Open5E labels this source "System Reference Document 5.2"; the curated
fields and implemented mechanics target the official 5.2.1 PDF. Original
download SHA-256:

`02B3D648E395CCEABDD238CE8AA1199B5C5B205A2F9F83E3A2DA93F97386EEEB`

The Bandit profile retains AC 12, HP 11, initiative +1, speed 30, scimitar +3
for 1d6+1, and light crossbow +3 for 1d8+1 at 80/320 feet. Damage types in
this response require checking the action text: the structured primary type is
null while the extra-damage-type field contains the type. The current module
does not yet resolve typed damage.

`vanguard`, `scout`, `adept`, and `healer` are authored combat fixtures, not
complete SRD class builds. `slums-orc` is an authored conversion selected for the
original Slums ORC identities. None of these profiles claims to be an imported
Open5E monster stat block. The party casters deliberately carry only two
level-1 slots for this standalone example. Shared campaign characters instead
use [manual level-1–4 advancement](../../../docs/ADVANCEMENT.md), including
level-two slots and selected supported spells/feats.

To update content, fetch an explicitly filtered source, retain the response and
provenance, compare supported fields with the official SRD, revise the curated
pack, and run the native tests. Unknown mechanics need implementation or an
explicitly authored fixture; they must not disappear silently in an importer.
The first milestone does not include a general importer or database dependency.

The header defines a format version and content revision. Each `creature` line
has the fields listed in the pack comment. `casting_bonus` includes proficiency
2 for these level-1–4 fixtures. Spell mask bits are Fire Bolt=1, Cure Wounds=2,
Magic Missile=4. The module identity includes a fingerprint of the whole pack
after CRLF normalization; changing definitions invalidates old checkpoints.

See [NOTICE.md](NOTICE.md) for the required attribution. SRD-derived content is
CC BY 4.0; the repository's code license does not replace that attribution.

Sources: [official SRD](https://www.dndbeyond.com/srd),
[SRD 5.2.1 PDF](https://media.dndbeyond.com/compendium-images/srd/5.2/SRD_CC_v5.2.1.pdf),
[Open5E API documentation](https://open5e.com/api-docs).
