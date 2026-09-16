# Spanish splash lettering

Generated with the built-in imagegen tool on 2026-09-15. The edit targets were
the approved English transparent overlays. Both selected PNGs are 1672 × 941
RGBA, copied unchanged into `art/`; no local image processing or warping was
performed. Corner alpha is zero. The background was not regenerated or edited.

## Engine overlay

Target: `art/OpenGoldBoxEngineLettering.png`

Prompt:

> Edit this transparent lettering overlay for the Spanish localization of the same game. Preserve the exact approved large beveled luminous gold Roman serif OpenGoldBox logo, its proportions, and composition. Replace only the ivory subtitle with this exact Spanish text on two centered lines: 'Un motor de código abierto para juegos de rol' and 'de la serie Gold Box.' Preserve the thin ivory serif style and modest dark local letter shadows. Background must be genuinely transparent alpha, no black rectangle, no checkerboard baked in, no scenery. This is lettering ONLY to overlay on a separate shared background. Match source canvas aspect ratio 16:9 and generous transparent margins; do not stretch or warp the lettering. Deliver transparent PNG.

Generated source: `C:/Users/imper/.codex/generated_images/01a0a759-0ed9-7300-a3ee-61d2e1fa4d28/exec-3fcc93fe-3796-4bf2-9dde-5d806e807b38.png`

Selected asset: `art/OpenGoldBoxEngineLettering.es.png`

SHA-256: `4ab5a0cae80161739f99cd59c8fc41b0b61676a7748e41d730e918fd0f24f5bd`

## Game overlay

Target: `art/OpenGoldBoxGameLettering.png`

The user explicitly selected **Estanque de Resplandor** for the Spanish title.

Prompt:

> Create the Spanish-localized version of this transparent gold game-title lettering overlay. The new exact title must be 'ESTANQUE DE' on first line and 'RESPLANDOR' on second line, in the same magnificent beveled metallic gold Roman serif style. Keep roughly the same central composition and generous margins, fit the longer letters naturally without warping or stretching. Exact ivory thin-serif subtitle below: 'Una adaptación no oficial creada con OpenGoldBox'. Exact smaller ivory disclaimer below: 'Sin afiliación ni respaldo de Wizards of the Coast.' Output LETTERING ONLY on genuinely transparent alpha background, no black rectangle or baked checkerboard, no scenery. Keep letter-local modest dark shadows, 16:9 canvas, fully visible lettering within margins. The separate unchanged shared background is supplied at runtime. Transparent PNG.

Generated source: `C:/Users/imper/.codex/generated_images/01a0a759-0ed9-7300-a3ee-61d2e1fa4d28/exec-7d7df4e3-0414-4129-a3cf-9d078859e6c8.png`

Selected asset: `art/OpenGoldBoxGameLettering.es.png`

SHA-256: `d34542584f4556b44aa02e71661148ad72134b4bd11df5005561eadd15e18afb`

## Shared backdrop

`art/OpenGoldBoxSplashBackground.png` remains byte-for-byte unchanged.
SHA-256: `70a9ad488839fb4b6633caf08d44df15ec19bfaf6c0aa227aa39e24fe70b6b54`.
The game fits it uniformly to the viewport and never swaps it when advancing
between splash screens. Text fades change the lettering alpha only.
