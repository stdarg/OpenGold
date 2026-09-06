# Asset identification: external source audit

Update: the script inspection and review integration are now implemented.
See [NPC art identification](npc-art-identification.md) for results and the
remaining runtime-validation limits. The audit below records the initial research.

## Result

The inspected tools provide a path to identify asset formats and many usage
relationships, but not a complete authoritative name for every game asset.
The most useful new lead is ECL: monster identity and combat icon identity are
separate command operands. Same-ID associations in our review tool remain
hypotheses, not the original game's full binding rule.

## Sources inspected

- [Gold Box Explorer](https://github.com/bsimser/Gold-Box-Explorer), revision
  `eac30abaa6ee66aea6f5d65ebe6d676b10015a8f`. Local reference checkout:
  `user-data/reference-gbe`. MIT-licensed repository; no source incorporated here.
- [Pool of Radiance JS](https://github.com/stlhood/pool-of-radiance-js), revision
  `8c8913ed0063c2e82763249ce4e468dfc2034057`. Local reference checkout:
  `user-data/reference-por-js`.
- [Gold Box Companion format documentation](https://gbc.zorbus.net/formats.zip),
  extracted under `user-data/gbc-formats`. Public Companion source was not
  located; its documentation was inspected instead. This is not a claim that
  Companion is open source.

## Findings from Gold Box Explorer

Paths below are relative to `src/Common/Plugins/` in the pinned repository.

| Area | Source | What it establishes |
| --- | --- | --- |
| Monster/icon selection | `DaxEcl/Commands.cs`, `CMD_LoadMonster` | Three operands: monster ID, number of copies, CPIC icon ID. Icon ID need not equal monster ID. |
| Encounter art selection | `DaxEcl/Commands.cs`, `CMD_SetupMonster` | Separate sprite ID, maximum distance, and picture ID. Encounter visuals can be chosen by scripts independently of combat monsters. |
| Pictures | `DaxEcl/Commands.cs`, `CMD_Picture` | Script-level picture selection is exposed by the disassembler. |
| ECL setup | `DaxEcl/Program.cs` | Registers LOAD MONSTER as 0x0B, SETUP MONSTER as 0x0C, PICTURE as 0x0E; explicitly selects memory origin 0x9900 for Pool of Radiance. Validate operand decoding before applying these to local scripts. |
| Combat images | `Dax/EgaBlock.cs`, `Dax/EgaVgaPalette.cs` | Header/pixel decoding and combat palette selection for CPIC, CHEAD, CBODY, COMSPR and combat terrain families. This is format classification, not individual monster naming. |
| Encounter and picture images | `Dax/EgaSpriteBlock.cs` | SPRIT transparency/palette behavior; PIC and FINAL later images are reconstructed using XOR with the first image. Our encounter decoder must not be blindly reused for PIC/FINAL. |
| Walls | `Dax/DaxWallDefFile.cs` | Assembles wall layouts from WALLDEF and 8X8D tiles, including shared tiles and multi-wallset block selection. Individual tiles need layout context to identify their role. |
| Monochrome graphics | `Dax/MonoBlock.cs` | Decodes 8x8 one-bit glyph/tile images. Meaning still requires usage context. |
| Image browser | `Dax/DaxImagePlugin.cs` | Selects image handlers and preserves block IDs; it does not provide a complete monster-name lookup. |

These are findings from a reference implementation, not proof that every
opcode/edge case behaves identically in our DOS release. In particular, do not
scan arbitrary bytes for 0x0B and interpret the following bytes as a binding:
script data, variable operands, branches, and resource context must be decoded.

## Companion documentation

`Character file formats/01. Pool of Radiance.txt` identifies these byte offsets:

| Decimal offset | Documented meaning |
| --- | --- |
| 108 | Icon dimensions |
| 159 | Type |
| 187, 188 | Portrait head, portrait body |
| 189, 190 | Icon head, icon body |
| 192 | Icon size |
| 193-198 | Body, arm, leg, hair/face, shield and weapon color selections |

This corroborates rejecting byte 159 as a CPIC archive selector. The head/body
fields provide a route to assembled character icons, but their interpretation
in MON records and the color packing still need validation. Do not apply these
fields to all monsters without distinguishing fixed icons from assembled ones.

## Why the JavaScript project is not the missing DOS mapping

`tools/convert_gamedata.py` explicitly targets extracted Apple II disk data and
uses a different name layout (null-terminated first 16 bytes). Our DOS monster
records have a length prefix. In `js/src/engine/CombatEngine.js`, `spawnEnemies`
creates hardcoded test enemy templates. This project does not supply a verified
complete DOS DAX monster-to-art mapping in the inspected extraction/runtime code.

## Recommended identification pipeline

1. Inventory every archive/record/image with format, dimensions and content hash.
   Retain source identity even when decoded images are duplicates.
2. Decode ECL instructions with Pool-specific addressing and operand handling.
   Record monster-load operands, encounter sprite/picture operands, script
   archive/record and instruction offset. Preserve variable references as
   unresolved instead of inventing constants.
3. Follow control flow and resource selection to resolve the selected archive.
   A script may reference multiple monsters and separate encounter artwork;
   proximity of commands alone does not prove a one-to-one relationship.
4. For fixed CPIC icons, combine those references with the locally checked
   base-ID/+128 image pairs. For assembled icons, validate head/body/color fields.
5. Follow GEO/WALLDEF references to give wall tiles their contextual roles;
   use picture-selection events to identify illustrations.
6. Retain user Incorrect decisions and name overrides. New automated evidence
   must not silently erase human review decisions.

Music, sound, unused records, ambiguous aliases, and dynamically selected assets
are not fully identified by this audit. No complete all-assets mapping was
found, and no speculative bindings were promoted into the demo.
