# Campaign saves

The OpenGoldBox rename preserves the original `user://` location using an explicit
custom user directory. Existing saves remain accessible without moving files.
The `OPENGOLD-CAMPAIGN` format identifier is unchanged for compatibility.

The shared character/party/New Phlan flow supports manual named campaign saves.
The first Slums expedition also supports saving during idle exploration, including
its district map, script continuation and deferred original loot. Format version
nine retains version-one through version-eight loading. It adds training choices
and grants to the existing exploration knowledge, active rules effects, sub-minute game time,
precise rest-completion times and encounter identities. See [status effects](STATUS-EFFECTS.md).
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
during combat or an unfinished town event. A completed Short Rest spending window
may be saved at the idle boundary. No autosaves, unfinished-script-request saves,
mid-combat campaign saves or original DOS saves are implemented. The standalone tour and combat research demos retain their existing
behavior; these campaign controls belong to the shared party flow.

Format **OPENGOLD-CAMPAIGN 10** stores:

- Finished character drafts, appearances, levels, advancement choices, training
  selections and acquired feature/feat/training grants,
  stable member and inventory IDs,
  inventory and original item provenance, equipment and selected grip, purses, NPC identities/morale,
  active/reserve membership, selected slot and character-pool candidate identities.
- HP/death state, opaque rules-owned resources, XP, claimed reward IDs, recovery
  timers, campaign minutes/millisecond remainder and service RNG state.
- Lasting effects in the rules-owned SRD3/FX1 continuation, including individual
  applications, source provenance, fixed DCs, remaining duration and recovery
  schedule. Encounter scope IDs distinguish reused monster IDs across fights.
- A completed Short Rest's spending ticket, eligible members and completion time,
  plus its next session ID. Reload continues after the last committed die without
  replaying the hour or recharge. Malformed/stale/inactive continuations reject.
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
Format 8 explicitly persists grant IDs, sources, acquisition levels and choices.
On load, those records must agree with the validated creation/advancement history;
missing, duplicate, forged or inconsistent records reject before replacing live state.
Formats 1–7 reconstruct the grant records from their existing history. No extra
feat is awarded and no previously spendable resource is refilled. Equipment and roster state are also
validated before replacement. Format 9 adds skill, tool, Expertise and language
selections and source grants. Older saves reconstruct fixed training grants while
leaving optional selections pending. Version 8 grants are validated against their
original feature/feat scope before reconstruction. See [training support](TRAINING.md).
Rules 0.6.3 replays Constitution/HP history and repairs the older low-Constitution
HP calculation while preserving wounds, zero-HP/dead state and spent resources.
Rules 0.6.4 also accepts 0.6.3 campaigns without reapplying HP repairs.
Rules 0.6.14 additionally accepts 0.6.4/0.6.5/0.6.6/0.6.7/0.6.8/0.6.9/0.6.10/0.6.11/0.6.12/0.6.13 campaigns. Standalone combat checkpoints
retain the post-attack action, Bonus Action, movement and spell usage; the
[0.6.4/0.6.5/0.6.6/0.6.7/0.6.8/0.6.9/0.6.10/0.6.11/0.6.12/0.6.13 combat migrations](RULES.md#library-boundary) cancel facing-only queues
and preserve genuine movement reactions. Format 7 also records involuntary
shared spaces after an allied-transit interruption; healing and recovery retain
valid checkpoints without moving actors or replenishing resources. Other combat module versions reject.
Campaign format 7 and combat format 8 retain weapon grip. Formats 1–6 migrate
Battleaxe, Spear, Quarterstaff and Trident to their previous two-hand use; other
Versatile weapons retain one hand. The alternate melee die is corrected under
the current rules, while wounds, resources and pending movement are preserved.
Frozen 0.6.6 campaign/combat files exercise both migration paths. See the examples in
[advancement](ADVANCEMENT.md).

Rules 0.6.10 preserves remaining Hit Dice in SRD4 vital continuations and combat
format 9. Older characters start with unspent dice because previous modules had
no spending operation. Campaign format 10 adds the spending continuation; formats
1–9 migrate with no pending entitlement. [Rest resources](REST-RESOURCES.md)
details the compatibility and currently supported scope. Rules 0.6.11 adds SRD5
and combat 10 mortality clocks without changing the campaign schema. Load never
rolls a recovery delay or invents elapsed recovery from old saves; see
[recovery clocks](RECOVERY-CLOCKS.md). Rules 0.6.12 advances those clocks alongside
effects during campaign elapsed time, with no further schema changes.

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
or protection against deliberate tampering. Formats 1 through 7 and the supported
rules 0.3.0–0.6.7 campaign identities migrate; see [manual advancement](ADVANCEMENT.md).

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

Rules 0.6.13 validates the previous grant schema before adding Dwarf Poison
resistance. PC8 makes that grant mandatory for new Dwarf combat recipes. The
exact preceding content pack migrates across additive damage metadata; wounds,
spent resources, RNG and mortality timing remain intact. See [damage](DAMAGE.md).

Rules 0.6.14 adds [Temporary HP](TEMPORARY-HP.md) in combat format 11 and SRD6
vital state. Campaign format 10 and PC8 remain unchanged; old saves gain no pool.

Rules 0.6.15 adds PC9's fixed Orc grant, SRD7's remaining Adrenaline Rush uses,
and combat 12's turn allowance and unresolved Temporary HP decision. Formats
through combat 11/module 0.6.14 migrate without resource refunds or RNG changes.
Campaign format remains 10 and continues to disallow saves during battle.
[Temporary HP](TEMPORARY-HP.md) documents exact boundaries and frozen fixtures.

Player save policy: save while camping or at an inn. Do not expose saving during
combat. Combat checkpoint codecs remain internal tools for deterministic testing
and continuation; their existence does not authorize an in-combat save control.

Rules 0.6.16 derives [Heavy weapon Disadvantage](HEAVY-WEAPONS.md) from existing
scores and equipment. PC9/combat 12/campaign 10/SRD7 are unchanged. Module
0.6.15 saves retain all grants, pools and expenditure; loading never rolls dice
or spends actions. Future affected attacks apply the corrected rule.

Rules 0.6.17 adds the [complete weapon catalog](WEAPON-CATALOG.md) without a
schema change. Existing original-item conversions, inventory IDs, quantity,
source records, equipment, HP, resources and clocks remain intact. In particular,
original type 45 retains the Fine Composite Long Bow / SRD Longbow mapping;
adding a native Heavy Crossbow does not justify reinterpreting old equipment.

## Armor catalog compatibility

Rules 0.6.18 retains the existing save schemas and accepts 0.6.17 campaigns and
combat-12 checkpoints. New armor keys use the existing equipment representation.
Frozen prior-writer fixtures verify that original armor provenance, grants,
wounds, resources and exact combat continuation are preserved. Armor penalties
are derived from the equipped definitions, including Chain Mail's Stealth
Disadvantage; loading never spends or restores resources. See [armor scope](ARMOR.md).

## Wizard spell knowledge

Rules 0.6.19 adds sourced cantrip/book grants to the existing ledger and writes
PC10 recipes. Casting access must agree with known cantrips and prepared book
entries. Campaign 10, combat 12 and SRD1–7 remain unchanged. Old campaigns
reconstruct the established Wizard preset and actual saved advancement choices;
unprepared learned spells remain in the book and unselected choices remain
pending. PC1–9 combat recipes retain their recorded access because those records
lack acquisition history. The frozen 0.6.18 writer proves exact continuation;
no load-time slot/use refund is allowed. Full selection controls and free casts
remain open. See [spell access](SPELL-ACCESS.md).
