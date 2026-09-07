# Character creation

The requested standalone demo will use the existing C++20/Godot GDExtension.
Its sequence is race, gender, class, alignment, attributes, HP, name, portrait,
combat appearance, then saving to a character pool. Player parties have at most
six created characters, leaving two positions for NPC hirelings.

The creation UI and character pool are pending these user decisions:

- SRD 5.2.1 race/class choices or original Pool of Radiance choices.
- Attribute rolling and assignment; rolled or maximum starting HP.
- Whether the first standalone demo includes party selection or only saves
  completed characters to the pool.

The art foundation below is implemented. There is no character-demo launcher
yet. This does not change the Rolf demo's fixed fighter or combat profiles.

## Original character art

`CharacterArt` in `OpenGold.Core` reads the installed game's DAX files, retaining
archive IDs and indexed combat pixels. No extracted artwork is distributed.

- Portrait heads use `HEAD1..8.DAX`; bodies use `BODY1..8.DAX`. The loader merges
  identical copies of an ID and rejects conflicting copies. It exposes the
  available original parts, without claiming a race/class-specific selection
  list. A head is 88 x 40 pixels, followed by an 88 x 48 body.
- Combat heads come from `CHEAD.DAX`, base IDs 0..13. Body/weapon components come
  from `CBODY.DAX`, base IDs 0..31. Add 64 for tall components and 128 for the
  action pose. Both head and body use the same size/pose bank. The head overlays
  the body's upper rows to produce a 24 x 24 icon.
- Source pixel 0 is transparent; source pixel 8 is an opaque black outline.
  Recoloring operates on source indices, so choosing black never makes a region
  transparent. The two poses share the same customization settings.

| Region | Color-1 source index | Color-2 source index |
| --- | --- | --- |
| Weapon | 6 | 14 |
| Body | 1 | 9 |
| Hair / face | 4 (hair) | 12 (face) |
| Shield | 5 | 13 |
| Arms | 2 | 10 |
| Legs | 3 | 11 |

Indices 7 and 15 remain fixed. Region names describe the original icon masks;
some weapon variants use shield pixels for ammunition or omit a region. All
six regions retain two independently selectable colors from the 16-color EGA
palette. Portrait colors are separate from combat-icon colors.

The assembly and mapping were checked against the installed DAX records. The
original manual's Icon menu describes independent head/weapon selection, two
poses and two sizes. Stephen S. Lee's
[binary-format research](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869)
also identifies the component archives and size bank. Its portrait-menu indices
are not used as an assumed list of available archive IDs.

## Verification

`build.cmd` and `build-rolf.cmd` include `opengold_character_tests`. Synthetic
tests cover truncation, all twelve color controls, head composition, fixed
pixels and transparency. Set `OPENGOLD_GAME_DIR` to the directory containing
the DAX files to additionally check every original head/body combination in
both sizes and both poses:

```bat
set "OPENGOLD_GAME_DIR=D:\path\to\POOLRAD"
build.cmd
```
