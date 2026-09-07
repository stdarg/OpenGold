# Pool of Radiance PC sound data

The sound board reads the user's `START.EXE` on startup. It unpacks EXEPACK
in memory, decodes the sound sequences, and synthesizes PC-speaker PCM in memory.
It never saves an unpacked executable, extracted sound data, or audio files.
No original executable instructions run in OpenGold.

## Sources and scope

- Stephen S. Lee's [PC 1.3 analysis](https://gamefaqs.gamespot.com/c64/578753-pool-of-radiance/faqs/73869)
  identifies the sound segment and gameplay effect selectors in section 14.
- The [EXEPACK format description](https://w4kfu.github.io/unEXEPACK/) describes
  the packed header and backward run-length encoding.
- Local inspection on September 7, 2026 establishes the data layout below for
  the Steam PC 1.3 executable. Later/other platform releases are unsupported.

The PC directory has 21 entries: silence (1), the eleven documented gameplay
effects (2–12), eight unused effects (13–20), and silent instrument setup (21).
The public game wrapper treats selectors 0/1 as output controls. The board
exposes directory entries directly, so entry 1 is labeled Silence.
This demo renders the PC-speaker bank. Tandy uses a separate directory and is
outside this demo's playback scope. This release has no music soundtrack.

## Data layout (offsets relative to the unpacked load image)

The sound segment starts at `0x8AE0`. All pointers below are relative to it.
The PC directory at `0x055F` contains 21 records of four little-endian 16-bit
sequence pointers. The Tandy directory follows at `0x0607`. Waveform,
envelope, and sequence data occupy the region through `0x1377`.
The playback timer divisor is read from `0x13A6`; its clock is the standard
PC PIT clock, 14,318,180 / 12 Hz. Each sequence tick lasts that divisor's
number of PIT cycles. Pitch values are PIT channel 2 divisors, not hertz.

## Sequence specification

Each voice has 24 unsigned 16-bit fields (byte-offset addressing):

| Offset | Meaning |
| --- | --- |
| 00 | Ticks until next commands; zero ends the voice |
| 02 | Next sequence position |
| 04 / 06 / 08 | Base divisor / per-tick delta / output divisor |
| 0A / 0C | Speaker gate / per-tick gate delta |
| 0E–1A | Note/envelope settings (only initialized by the silent setup entry) |
| 1C / 1E | Modulation table pointer / phase in sixteenths of a byte |
| 20 / 22 / 24 | Modulation phase step / signed depth / period |
| 26–2E | Loop counters |

Commands used by this PC bank:

| Byte | Payload | Meaning |
| --- | --- | --- |
| FF | field:u8, value:u16 | Assign a field; assigning 00 yields for that many ticks |
| FE | field:u8, target:u16 | Decrement a nonzero counter; branch unless it reaches zero |
| FC | target:u16 | Call a sequence subroutine (one return slot) |
| FB | — | Return |
| FD | voice-offset:u16 | Select/reset synthesis fields of a voice; offsets are multiples of 48 |

At a tick, active voices advance gate and base divisor by their respective
deltas with 16-bit wrapping. Modulation phase advances, subtracting its period
once on wrap. The signed table byte at phase/16, multiplied by 256 and signed
depth, contributes the signed high word of the product to the output divisor.
Then the tick counter decrements; reaching zero processes commands until a
yield/end. Field assignments affect output on the next tick, while gate changes
apply immediately. The first active voice with a nonzero gate supplies the PC
output; gate bits 0 and 1 must both be set for a tone.

The native reader validates the supported image, all reads/fields/targets,
command budgets and duration limits. It rejects unsupported sequence commands
instead of inventing replacement sounds. A separate Core renderer converts
the resulting ticks to 48 kHz mono PCM, averaging the square wave within each
sample. Hardware speaker coloration and phase at the instant of an original
interrupt are not reproduced; no acoustic recording comparison is claimed.

## Run and verify

From PowerShell:

```powershell
.\build-rolf.cmd
.\review-sounds.cmd
```

The demo uses `OPENGOLD_GAME_DIR`, falling back to `opengold/game_directory`
in `godot/project.godot`. Set the variable to the folder containing `START.EXE`.
Click a sound to play/restart it; a new selection stops the previous sound.
Stop, mute and volume are available below the buttons.

The native sound tests use hand-authored fixtures, plus optional inspection of
all 21 entries when `OPENGOLD_GAME_DIR` is set. Godot's `--sound-check` mode
exercises each button's pressed signal and verifies playback starts and ends.
