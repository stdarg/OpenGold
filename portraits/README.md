# Complete character portraits

Six independently composed portraits form the first style-review sample of a
planned 36-portrait collection. They are pending visual approval; the remaining
30 portraits and game integration are outside this review step.

Each PNG contains a complete chest-up character on a black background. Faces,
physiques, poses, and clothing are individually designed rather than assembled
from interchangeable heads and bodies. The sample includes two Female, two Male,
and two Nonbinary characters; the Nonbinary briefs request androgynous presentation.
Real-world ancestry-inspired facial and hair characteristics vary across fantasy
races and are not game-rule categories or metadata fields.

`portraits.json` maps each PNG basename, including its extension, to exactly
`Class`, `Gender`, and `Race`, using the character creator's display names.
`prompts.json` records the exact prompts used with the built-in image generator.
The original game's local contact sheet informed general palette, framing, and
readability; no extracted original assets are included here.

All six PNGs retain the generator's 1254 x 1254 full-resolution output. Pixel-art styling does
not guarantee a mathematically uniform low-resolution pixel grid. Local review
previews render each at 88 x 88 as well as enlarged sizes; these are visual
readability checks, not a claim of tested runtime integration or a separately
hand-tuned 88 x 88 asset set.

Local review sheet: `../user-data/portrait-review/review-sheet.png`. It shows
352 x 352 previews, native 88 x 88 previews, and those small previews enlarged
2x with nearest-neighbor sampling. All six catalog entries, image dimensions,
and unique file hashes were checked; the large and small previews were visually
inspected. Final aesthetic acceptance remains with the user.
