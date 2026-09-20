# Combat artwork assignments

Run from PowerShell at the repository root:

```powershell
.\demos\review-combat-bodies.cmd
```

The reviewer displays one original CBODY.DAX body in four previews: short/tall,
ready/action. Previous/Next cycles through all 32 bodies. Check every complete
combination that the artwork represents, such as Mace and Mace & Shield.
Each checkbox change saves immediately. Filtering only changes the visible
checklist; the summary always lists all assignments for the current body.
An empty assignment is visibly Unreviewed. Original game files are decoded
locally and are not distributed.

## Shared catalog

`data/art/combat-body-looks.tsv` has 32 rows: body ID, a tab, then comma-separated
combination IDs (or `unreviewed`). For example:

```text
1	type_41,type_42,type_43,type_44,type_45
```

The options in `data/art/combat-weapon-options.tsv` describe ordinary shop
weapons and Unarmed, each with an optional `_shield` combination. Silver weapons
share ordinary associations. Both the game and reviewer accept the old single
ID format and normalize legacy `silver_N` IDs to `type_N`, preserving shield
suffixes and deduplicating associations. The next checkbox edit writes the
whole catalog in the new format. Saves use a temporary file and replacement;
a failed save keeps the previous assignments.

The original bow body 1 has been visually inspected in both poses and sizes
and shared between the bow options. Other existing assignments are retained;
unclassified bodies remain unreviewed. These are editable art classifications,
not changes to equipment rules.

## Game use

Combat matches the exact equipped weapon and shield combination. If several
bodies match, the character's saved body wins if it is among them; otherwise
the lowest matching ID wins. An unmatched combination retains the saved body
and is reported in the combat log. Only the body ID changes; the head, colors,
size and both poses are preserved. Silver names do not select separate art.

Rebuild the game after editing to refresh its packaged catalogs. The reviewer
always edits the shared source catalog, not a packaged copy.
