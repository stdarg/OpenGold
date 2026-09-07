# New Phlan exploration demo

The [party preview](PARTY.md) can inject created PCs/NPCs into this same native
host, including shared purses/inventory, WHO and character/party queries.
The standalone launcher retains the single-fighter fixture described below.

`review-rolf.cmd` runs the C++/Godot Rolf tour followed by free exploration of
the complete 16 x 16 New Phlan map, including building locations. The current
scope is the civilized town; ruined districts and boat destinations are outside
this host. A fixed level-1 fighter starts with 12/12 HP, 9,999 gold pieces and
an empty inventory. Combat rules remain the separate SRD 5.2.1 module.

## Playing

From CMD in the repository root, build once with `build-rolf.cmd`, then run
`review-rolf.cmd`. Existing game-directory configuration is unchanged.

- Complete Rolf's eight dialogue pauses with Continue/Enter.
- Arrow keys or the turn/step buttons move the party; stepping crosses ordinary
  doors. Solid walls, locks, and script-imposed restrictions still apply.
- Look (L) runs the search entry at the current position and facing.
- Select an original dialogue choice, then Choose/Enter. Numeric/string prompts
  accept a typed answer followed by Submit/Enter.
- Accept a shopkeeper's offer, select merchandise, and Buy/Enter. The list
  scrolls. Leave shop resumes the original script and restores movement.
- Inventory shows purchased items and gold. Replay tour resets the whole demo,
  including purchases, flags and money. Closing the application does not save.

## Script and state boundary

The shared, immutable resource profile contains `GEO3.DAX:0`, the verified Phlan
wall bank, and `ECL3.DAX` records 0 (town), 8 (City Hall) and 11 (training hall).
Scripts themselves request the building transitions; coordinates are not used
to replace original dialogue or choose a handcrafted event.

The host schedules slot 0 before resolving a forward step, then slot 1 after
resolution. Turning and Look invoke slot 1. `NEW ECL` preserves campaign state,
clears area-local flags through the VM and schedules slot 4 followed by slot 1.
The native camp command exposes the pre-camp entry, but does not perform a rest.
This is a bounded adapter policy, not a verified reproduction of every DOS
main-loop detail. Time currently stays at midday.

Logical campaign/local/register/scratch ranges are initialized explicitly.
LOAD CHARACTER and WHO expose the single fighter and empty remaining party
slots. Known name, status, HP, Constitution and purse fields are mapped; this
is not a complete character-record or SRD character-sheet adapter. Inventory
and purse use native value ownership. The script character view is synchronized
at character switches, event completion and shop exit. While shopping, the VM
is suspended and the native purse owns transactions.

Each location invocation keeps a native checkpoint. Unsupported engine services
produce a visible notice and a retained diagnostic, restore script/party state,
and allow Continue back to exploration. They are not reported as successful
combat, training, healing or quest completion. Original scripts can still deny
entry or move the party out of restricted rooms.

## Original merchandise and artwork

| Shop | Example map position | `ITEM3.DAX` record | Stock entries |
| --- | --- | --- | --- |
| General supplies | (15,8) | 54 | 7 |
| Jewelry | (8,10) | 52 | 11 |
| Arms and armor | (13,8) | 53 | 57 |
| Silver items | (11,10) | 55 | 13 |

Original TREASURE instructions select these lists. The COMBAT command opens a
shop when script register `6E6C` is set. Closing resets that flag and resumes the
pending instruction. Each purchase charges the decoded item's gold value and
adds its stored item/bundle to inventory. Stock may be purchased repeatedly.
The demo has a 16-entry inventory limit and rejects invalid/stale requests,
unaffordable purchases and purchases when full without charging gold.
Selling, equipping, item activation and currency exchange are not implemented.

HEAD3/BODY3 records provide 88 x 40 heads and 88 x 48 bodies, combined according
to the script's portrait selection. PIC3 and SPRIT3 supply supported pictures
and encounter sprites. All images and items load from the user's installation;
none of the original bytes, pictures or text are bundled in this repository.

Mapped registers and shop/treasure dispatch were cross-checked with the
[PC 1.3 technical analysis](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869),
then exercised against the installed ECL and item records. Dialogue, stock
counts and item prices are taken from those records, not copied from the guide.

## Validation and remaining work

Native synthetic tests cover buying, insufficient funds, capacity, stale
requests, replay, VM purse synchronization, unsupported-event rollback and
picture bounds/palette. With `OPENGOLD_GAME_DIR` set, the suite attempts routes
to every numbered cell. The current peaceful-choice run reaches 56 event cells
covering 30 distinct event IDs and all three town programs, and separately buys
an affordable item from each of the four shop types. Restricted routes and
unsupported branches are reported, not counted as successful execution.

The Godot check completes the tour, uses movement/button callbacks to reach
the arms shop, purchases a shield, opens inventory and leaves the shop:

```cmd
review-rolf.cmd --headless -- --town-check
review-rolf.cmd -- --town-check --capture
```

Captures are written under ignored `user-data/phlan-shop.png` and
`user-data/phlan-inventory.png`. Tested at 960 x 720 and 1280 x 900.

Unimplemented branches include town combat/duels and their party-strength
queries, training, temple spell services, resting, robbery, recruitment,
non-shop treasure awards, external travel and saved-game persistence. Campaign
flags affect which dialogue branches can be reached; this test is not exhaustive
coverage of all choices, quest states, random outcomes, or camp interruptions.
