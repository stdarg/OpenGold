# Campaign saves

The OpenGoldBox rename preserves the original `user://` location using an explicit
custom user directory. Existing saves remain accessible without moving files.
The `OPENGOLD-CAMPAIGN` format identifier is unchanged for compatibility.

The shared character/party/New Phlan flow supports manual named campaign saves.
The first Slums expedition also supports saving during idle exploration, including
its district map, script continuation and deferred original loot. Format version
six retains version-one through version-five loading. It preserves exploration
knowledge and adds active rules effects, sub-minute game time, precise rest
completion times and encounter identities. See [status effects](STATUS-EFFECTS.md).
Adding the supported roaming creature
profiles preserves compatibility with the preceding rules content pack; unrelated
content changes still require a matching identity.
Use **Save game** or **Load game** on the party roster or during idle town
exploration. Select an existing slot or enter a new name. Overwriting requires
confirmation and retains a previous version; loading always asks before replacing
the current campaign. The load list also offers previous versions for recovery.

Loading returns to the party roster. **Explore New Phlan** resumes the saved town
position, facing, visited map and script state. A save made before first entering
town resumes before Rolf's tour. Unfinished character-creator drafts are not part
of a campaign save; add completed characters to the party first.

## Supported boundaries and state

Saving/loading is supported from the party roster and idle New Phlan exploration.
Town controls are disabled while dialogue, input, shopping or services are pending.
Combat must finish and return to the roster first. The core also rejects saving
during combat or an unfinished town event. No autosaves, pending-request saves,
mid-combat campaign saves or original DOS saves are
implemented. The standalone tour and combat research demos retain their existing
behavior; these campaign controls belong to the shared party flow.

Format **OPENGOLD-CAMPAIGN 6** stores:

- Finished character drafts, appearances, levels, advancement choices,
  stable member and inventory IDs,
  inventory and original item provenance, equipment, purses, NPC identities/morale,
  active/reserve membership, selected slot and character-pool candidate identities.
- HP/death state, opaque rules-owned resources, XP, claimed reward IDs, recovery
  timers, campaign minutes/millisecond remainder and service RNG state.
- Lasting effects in the rules-owned SRD3/FX1 continuation, including individual
  applications, source provenance, fixed DCs, remaining duration and recovery
  schedule. Encounter scope IDs distinguish reused monster IDs across fights.
- Current New Phlan script/resource context, private mutable ECL image, bound
  variables/flags, instruction spans, comparison flags, request counter and ECL
  RNG. The completed event is not replayed. Dialogue and visited cells persist;
  transient encounter pictures/sprites are cleared for the idle exploration view.
- Separate visited and seen bitsets for each district. Looking ahead through the
  first-person view reveals visible cells permanently; changing maps or loading
  a save preserves that history. Older saves retain their visited cells and add
  the current sightline when displayed. Invalid masks, unknown districts, or
  missing knowledge for visited cells reject the load.

The `--no-fog` launch flag only changes overhead rendering. It does not fill
either bitset, and saving while it is enabled does not reveal unexplored areas
when the save is later loaded during normal play.

Character sheets are reconstructed through the rules module by replaying validated
advancement choices. Levels 1–4 of the Fighter/Cleric/Wizard subset, selected
feats/spells and spent level-one/two resources persist; see [advancement](ADVANCEMENT.md).
Equipment and roster state are validated before replacement.
Rules 0.6.3 replays Constitution/HP history and repairs the older low-Constitution
HP calculation while preserving wounds, zero-HP/dead state and spent resources.
The campaign format remains 6; see the migration examples in
[advancement](ADVANCEMENT.md).

## File safety and compatibility

Saves live in Godot's writable **user://saves** directory (normally
`%APPDATA%\Godot\app_userdata\OpenGold\saves` on Windows). UTF-8 slot names are
encoded into safe filenames, with `.ogs` and previous-version `.ogs.bak` files.
Original installations may remain read-only.

The loader checks the format version, rules module/version/content identity and
an ordered fingerprint manifest of installed DAX archives and ITEMS. A different
asset installation, unknown definition, malformed resource state or incompatible
version rejects explicitly. Reinstalling identical assets at a new path is valid.
FNV-1a fingerprints/checksums detect accidental changes; they are not signatures
or protection against deliberate tampering. Formats 1 through 5 and the supported
rules 0.3.0–0.6.2 campaign identities migrate; see [manual advancement](ADVANCEMENT.md).

Writes use a temporary file, flush it to disk and verify its bytes before replacing
the destination. Windows uses `ReplaceFileW` with a retained backup, or
`MoveFileExW` for a new slot. Failed replacement leaves the existing save intact.
Truncated/corrupt loads and validation failures preserve the current campaign.
Each write uses a separate temporary filename and removes that file on failure;
files left by interrupted writes do not block later saves. The game's training
combat saves use the same storage service with their own size limit and codec.
Temporary files are ignored by the slot list. If the newest save is damaged,
explicitly choose its previous version in **Load game**.

## Verification

From PowerShell:

```powershell
.\demos\build-rolf.cmd
.\demos\review-character.cmd --save-restart
```

The launcher uses an isolated profile under `user-data/save-check-profile`, writes
fixtures under `user-data/save-check`, exits the writer process, then starts a
fresh Godot reader. The writer runs the existing original-data party route and
saves after advancement, interrupted rest, cancelled temple service, temple
payment, inn rest, a rejected full-health healing request, denied repeated rest,
and subsequent combat. The full-health rejection invokes the native campaign
service directly; the other original service paths use existing Godot callbacks.
The reader compares all serialized state, checks reward claims/rest eligibility,
verifies corrupt-load rollback and exercises named-slot save/load confirmations.

`opengold_save_tests` writes actual files and covers inventory IDs, reserve members,
HP/resources, levels, money, reward/rest continuity, ECL continuation, deterministic
subsequent combat and service RNG, incompatible identities, malformed/truncated
files, backup rotation and failed Windows replacement. The existing nine native
suites remain in the build alongside this tenth suite.

For visual checks, after the restart route:

```powershell
$env:APPDATA = Join-Path $PWD 'user-data/save-check-profile'
godot --path demos/godot --resolution 1280x900 res://scenes/character_creation.tscn -- --save-check-read --capture
```

Run the visual command from a temporary PowerShell session so its profile override
does not affect later interactive launches. Captures go to `user-data`; repeat at
1120x800. The tested Windows build uses Godot 4.7.2 and the repository's C++20
MSVC/GDExtension configuration.

This completes the bounded existing-flow persistence milestone, not the complete
expedition in issue #1. Issue #10 retains future class-feature/training coverage
and integration with the expedition's reward/quest state.
