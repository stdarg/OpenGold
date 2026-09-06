# Original game artwork

See [asset-source-audit.md](asset-source-audit.md) for the external source review:
ECL has separate monster/icon references, encounter picture/sprite selection,
and the available character-format notes describe assembled icon fields.

## Encounter sprites: `SPRIT*.DAX`

The demo's **Encounter sprites** category displays first-person encounter
sprites, not tactical combat icons. Their different sizes represent creatures
at near, medium, and far encounter distances. They are separately drawn distance
variants, not successive standing/attacking animation poses.

For example, `SPRIT1.DAX` record 2 in the inspected Pool of Radiance installation
contains:

| Stored image | Native dimensions | Purpose |
| --- | --- | --- |
| First / largest | 48 x 80 pixels | Near encounter |
| Second / medium | 32 x 65 pixels | Medium-distance encounter |
| Third / smallest | 24 x 49 pixels | Distant encounter |

Dimensions vary by creature; these example sizes are not universal. The
[Gold Box Games Wiki](https://wiki.goldbox.games/index.php/Pool_of_Radiance)
describes the game's short-, medium-, and long-range sprite variants.

The inspected installation has eight `SPRIT` archives containing 88 records
and 258 stored images. These counts describe this installation's encounter
artwork, not the total number of combat sprites in the game.

The decoder and demo currently call each stored image a "frame". This is a
storage/indexing term here; it should not be interpreted as an animation frame.
The demo browses all of these images with Left / Right and compares
nearest-neighbor, xBR level 2, and HQ4x at 4x scale.

### How encounter sprites are used in the game

Encounter sprites appear in the first-person exploration window when the party
meets a creature or group. They place the creature in the scene before tactical
combat begins, including encounters where the party can approach, talk, or fight.
The available choices depend on the encounter.

The game selects the small, medium, or large artwork according to encounter
distance. Switching from a smaller variant to a larger one can convey the
creature approaching the party. This is a change in apparent distance, not a
standing-to-attacking animation. The separately drawn variants preserve readable
details at their intended sizes.

If the encounter leads to combat, the display switches to the tactical
battlefield and uses separate combat icons. The encounter sprite is not reused
as the battlefield character.

The current demo is an asset comparison viewer: it displays each stored variant
individually at 4x scale. It does not yet reproduce sprite placement within the
exploration scene or select variants based on the party's encounter distance.

## Tactical combat icons: separate assets

The combat icons expected to have standing and attacking poses belong to a
different asset set. Inspection of the first records in `CPIC1.DAX` found
individual 24 x 24 pixel images, with one image per inspected record.

These assets use a different record header and combat palette from the
`SPRIT` encounter images. Gold Box Explorer's
[EgaBlock decoder](https://github.com/bsimser/Gold-Box-Explorer/blob/master/src/Common/Plugins/Dax/EgaBlock.cs)
recognizes `CPIC`, `CHEAD`, `CBODY`, and `COMSPR`, among others, as combat-palette
resources. Its
[EgaSpriteBlock decoder](https://github.com/bsimser/Gold-Box-Explorer/blob/master/src/Common/Plugins/Dax/EgaSpriteBlock.cs)
handles the separate encounter-sprite layout.

The demo now defaults to **Combat sprites**, loading every stored image from
`CPIC*.DAX`, `COMSPR.DAX`, `CHEAD.DAX`, and `CBODY.DAX`. In the inspected
installation this includes 428 images across 11 archives. Head/body components
are displayed individually, not assembled into a customized character.
The dropdown switches to **Encounter sprites** for the 258 `SPRIT` images.
Both categories support Left / Right navigation and retain their browsing
positions. All three rendering styles display the same selected image.

Combat poses stored as separate records are accessible through navigation;
they are not yet paired or automatically animated. The earlier missing combat
pose was caused by browsing encounter assets only, not by skipping a `SPRIT`
image.

### Still to verify

See [monster-art-mapping.md](monster-art-mapping.md) for the initial investigation,
reproducible CSV generator, candidate names, and the checked ID+128 pose-pair
relationship. Monster bindings and runtime archive selection remain provisional.

- How combat records map to creatures and pair into standing/attacking poses.
- Whether pose pairing and dimensions are consistent across all `CPIC` archives.
- The roles and relationships of the other combat-art resources before including
  them in a claim to support all combat artwork.

Do not assume that adjacent record IDs form an animation pair until verified.

## Correction to earlier project descriptions

Earlier demo descriptions called the `SPRIT` images "combat sprites" and claimed
that all combat sprites were loaded. That identification was incorrect. The
demo now distinguishes the two categories and additionally decodes the combat
archives listed above. Older descriptions of the encounter-only demo may still
use the inaccurate terminology.

See [graphics-format.md](graphics-format.md) for the currently implemented
encounter-image storage layout. Its historical "combat sprites" terminology
should be read with the correction above.

## Script-based NPC identification

The review tool now includes script evidence for NPCs and shared human art.
See [NPC art identification](npc-art-identification.md) for the remaining-group
findings, the tavern-brawl table mapping, and runtime verification limits.
