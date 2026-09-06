# Rolf's welcome: design for review

Status: proposed interface and implementation plan. The accompanying interactive
mockup does not execute ECL or implement campaign gameplay.

## Player experience

Keep the first-person view dominant, with a smaller north-up map alongside it.
Rolf appears inside the world view. Dialogue sits directly below the scene so
reading and continuing do not obscure either his sprite or the map.

| Element | Proposed behavior |
| --- | --- |
| First-person window | Show the street from the party's cell and cardinal facing. Composite the encounter sprite over the wall/background layers. |
| Local map | Show explored geometry, walls and doors; a distinct triangular party marker points in the party's direction. Keep north at the top while the marker rotates. |
| Dialogue | Show Rolf's text with one Continue action. Acknowledge the current script request; do not invent Accept/Decline choices. |
| Movement | Turn left/right and step forward when exploration permits it. Disable these controls during dialogue and scripted movement. Discard those inputs instead of queuing them. |
| Scripted tour | Remove Rolf when instructed, update party position/facing, redraw both views, then present the next text request. |

The desktop starting layout allocates roughly two thirds of the content width
to the scene/dialogue and one third to the map. The preview provides design
controls for map width (28–45%), explored/full-area visibility, a facing cone,
Rolf's stored distance variant, and welcome/first-stop/exploration states.
These controls belong to design review, not the proposed game's interface.

On narrow surfaces, put the scene and dialogue first and the map below. The
Godot implementation should use containers and minimum sizes, with integer
scaling/letterboxing of source artwork rather than stretching it to fill a panel.
Use nearest-neighbor filtering by default. Keep text at the UI's independently
adjustable scale and ensure all actions work by keyboard and mouse.

The proposed player map starts in explored mode. This review starts with the
full synthetic area visible so the layout is easy to assess; explored mode is
available through its design controls. Full-area visibility is a debug option.
The directional cone is an orientation aid, not a claim to implement visibility
or line of sight. Production exploration discovery needs its own verified policy.
Keep script event IDs and hidden destinations out of the player map. Selecting a
map cell must not teleport the party or dispatch ECL.

## Review mockup and provenance

The source is [rolf-welcome.html](rolf-welcome.html). It contains only original
mockup code, a synthetic 16×16 street layout, and newly authored sample dialogue.
Its two stages illustrate welcome → movement → first stop; they are not a
transcription of the tour, its timing, its coordinates, or its triggering rules.
The first stop intentionally ends the preview. It does not release control in
the actual campaign. The separate Explore layout design option demonstrates
synchronized movement and collision against the synthetic layout.

The scene uses a small ray-cast stand-in so its walls agree with this synthetic
map. That is a review convenience; the proposed game renderer below uses the
original perspective pieces. The mock's explored mask reveals a small radius
around visited positions; it does not model occlusion. Neither simplification
should become a gameplay rule by accident.

To include the encounter art from the configured local installation:

```powershell
godot_console --headless --path godot --script ../tools/preview_rolf_welcome.gd
```

This creates `user-data/rolf-welcome.html`, embedding the stored images from
`SPRIT3.DAX:12` using the existing `DaxSpriteLoader`. It uses `OPENGOLD_GAME_DIR`
or the development setting in `project.godot`. The output stays ignored because
it contains derived original-game art. Without the archive, the helper produces
a labeled placeholder. Malformed available records fail explicitly.

The source fragment can also be viewed directly without local art. Its primary
Continue interaction works without the conversation's optional design controls.
The preview makes no network requests and never changes original game files.

Evidence we can use now:

- `ECL3.DAX:0` contains Rolf's introduction at `0xB0B9`. The preceding sequence
  includes `SETUP MONSTER` at `0xB0AD` with sprite operand 12, delays and approach
  requests. [NPC art research](../npc-art-identification.md) associates this with
  `SPRIT3:12`; runtime bank selection is still provisional.
- The introduction invokes the Continue-prompt helper at `0xAF1F`. The subsequent
  sequence clears the sprite/picture and reads the tour's tables. The first
  landmark text is at `0xB1C5`.
- [MAPS.md](../MAPS.md) documents the native GEO loader and perspective-wall
  pipeline. The reference map-name table labels `GEO3.DAX:0` as New Phlan; matching
  record numbers alone do not establish runtime map-to-script dispatch.

Before making the tour playable, capture original-game entry conditions, active
map and resource banks, coordinate/facing conventions, screen composition,
approach distance transitions, state writes, and the first-stop sequence. Record
game version and asset identities. Test whether and when the welcome repeats;
do not infer one-time behavior from a guessed meaning for a flag.

## Proposed implementation

### One session owns the state

Add a native `PorSession` owning the active map/resource context, party pose,
the ECL machine, event scheduling, and pending presentation state. Use value
types and RAII containers for ownership. Share only genuinely immutable catalogs
through `std::shared_ptr<const ...>`; use `std::unique_ptr` if exclusive pointer
ownership becomes necessary. No raw owning pointers or Godot nodes in the core.

The map and first-person renderer must consume the same immutable presentation
snapshot, identified by a monotonically increasing revision. A proposed snapshot
includes map identity, party cell/facing, discovered cells, resolved wall-set
identities, encounter art identity/distance, dialogue, pending request ID, and
permitted inputs. This is a proposed contract, not a committed API.

Extend the current ECL binding mechanism with checked mapped-state access so
script writes to position/facing update the authoritative world state. Validate
all writes before committing a group of mutations, derive the current cell data,
then publish one snapshot. Do not copy independently writable position variables
into both `EclMachine` and `PorSession`.

### Godot presentation

Proposed scene structure:

```text
ExplorationScreen (Control)
  LocationBar
  Content (HBoxContainer; stacked at narrow widths)
    SceneAndDialogue (VBoxContainer)
      ViewFrame (AspectRatioContainer)
        ExplorationView (Control / CanvasItem)
      DialoguePanel
    MapAndCommands (VBoxContainer)
      AutomapView (Control / CanvasItem)
      PositionAndFacing
      MovementControls
```

A small GDExtension `PorSession` wrapper exposes commands, bounded advance,
validated input replies, and immutable presentation snapshots. It retains native
session lifetime across frames. Continue submits the current request ID, disables
itself immediately, and waits for the next snapshot. Duplicate or stale replies
must not advance another prompt or move the party twice.

Do not use the inspector's one-shot JSON subprocess for per-frame gameplay.
Keep that inspector and the console ECL harness as separate research tools.

### First-person rendering

1. Decode the tile and wall definition resources already described in
   [MAPS.md](../MAPS.md): `8X8D*.DAX` plus `WALLDEF*.DAX`.
2. Resolve tile banks and selected wall sets from the active resource context,
   including verified `LOAD PIECES` behavior. Preserve complete archive/record
   identities instead of inferring banks from matching IDs.
3. Assemble and cache each native perspective piece once per resource identity.
   Retain transparency and the source dimensions.
4. Convert nearby GEO edges to view-relative front/left/right slots using party
   position and facing. Select the corresponding distance and wall/door pieces.
   Validate side-edge ordering and asymmetric GEO edges before general movement.
5. Draw the background and wall/door pieces in the verified occlusion order,
   then the encounter sprite in its designated foreground layer. Door state,
   indoor/outdoor background choice, and exact sprite anchoring need evidence.
6. Present the composed logical view at integer scale inside a letterboxed
   rectangle. The mock's 320×200 canvas is illustrative, not a verified original
   first-person viewport size.

Rolf's near/medium/far artwork consists of separately drawn distance variants.
Switch the stored variant according to validated encounter state; do not animate
them as standing/attacking poses or stretch one image to simulate every distance.
Keep the art selection separate from creature identity. Showing Rolf's sprite
does not by itself require creating a combatant through `CreatureFactory`.

### Map, movement, and scripts

Reuse `MapCatalog` and `GeoMap` for the map view, with a separate party marker and
discovery overlay. Geometry comes from the loaded map; party location comes from
the same session revision as the first-person view. For a transition, stage the
new map, script and graphics, validate them, and publish the swap together.
Discard stale triggers from the old area.

During free exploration, send a movement intent to the core. The core applies
verified wall/door rules, the appropriate ECL entry scheduling, position and time
changes, and presentation requests. During the tour, accept only the script's
commands and acknowledgements. `DELAY` belongs to presentation scheduling and
must not sleep the main thread or implicitly advance game time.

Implement only the host capabilities reached by the verified first-stop path.
Likely dependencies include resource loading, sprite setup/approach/clear,
picture clear, delay and native movement/redraw services. This candidate list
must be checked against an execution trace; static adjacency is insufficient.
Do not enable a capability until its concrete side effects are implemented.

## Delivery and acceptance

1. **Review this design.** Decide scene/map proportions, map visibility, dialogue
   placement, and whether the facing cone earns its space.
2. **Build the synchronized view harness.** Use generated geometry and art to
   verify all four facings, turns, blocked moves, door edges and consistent
   scene/map revisions without needing proprietary test assets.
3. **Implement original wall assembly and the minimal session host.** Compare
   rendered native-size pieces and first-person views with recorded source-game
   observations. Test the verified distance selections and clear operations.
4. **Connect the original welcome through the first stop.** Compare initial
   state, request order, text, movement, facing and final state against the
   original-game trace. Test input locking, repeated clicks, bounded execution,
   resource failures, and map transitions without stale frames or triggers.

Save/load is a follow-on once the slice works. A later snapshot contract must
preserve the pending request, VM private image, RNG, resource identities and
world state so restoring a prompt does not replay completed movement or effects.

The design does not claim to implement the full tour, campaign persistence,
combat, original movement scheduling, or original-game compatibility.
