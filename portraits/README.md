# Complete character portraits

Original AI-generated painted fantasy portraits for OpenGold, made with the built-in image generation tool. These are complete portraits, not interchangeable heads and bodies, and are not extracted from Icewind Dale or Pool of Radiance.

Target: native square PNGs at least 1024 pixels per side, suitable for the character sheet and later party-icon cropping. Preserve the original generated files at their native resolution.

`plan.json` defines the coverage matrix: nine species, three genders, twelve classes, and seven appearance groups for humans, elves, dwarves and gnomes. This is 1,188 planned unique portraits. Only existing PNGs are completed artwork; the plan is not a completion claim.

Each generated portrait has a matching JSON sidecar containing its authoring metadata and prompt. Gender and appearance tags describe the requested fictional character and are not inferred from the image. Latino backgrounds span multiple ancestries; these categories are broad art-direction aids, not biological rules or restrictions on player choice.

Artwork generation is separate from integration. The current game still uses its existing portrait implementation until that change is authorized and implemented.

`portrait-index.csv` lists completed portraits with five columns: portrait file name, class, race (in-game), racial base, and gender. Racial base follows the authoring metadata; `Fantasy species variation` means no specific real-world racial base was assigned. The index is refreshed at generation checkpoints.
