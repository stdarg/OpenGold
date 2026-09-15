# Full HD splash generation prompts

Built-in image generation was used. The final background is derived from the
fresh engine composition below by removing its lettering. Both runtime screens
share that single resulting PNG; their text is rendered separately in Godot.
The requested canvas was 1920 x 1080; actual generated dimensions were
1672 x 941. No local resampling, cropping, or warping was applied to the source.

## Fresh composition prompt

Generate a NEW original production game splash screen. Mandatory output canvas: exactly 1920 pixels wide by 1080 pixels high (Full HD 16:9 landscape). Compose the scene natively for this canvas. High-detail painterly fantasy illustration: an ancient dark stone dungeon wall, a burning iron torch and burgundy red banner with muted gold sun emblem on the left, an ominous dragon-shaped shadow across the upper right wall, luminous blue water along the bottom with dark foreground rocks. Warm amber lighting on the left contrasts with cool deep blue lighting on the right. Centered large elegant beveled gold classical serif main title exactly 'OpenGoldBox' on one line around 40 percent height. Centered warm ivory serif subtitle below on two balanced lines exactly 'An open-source role-playing game engine for Gold Box games.' Typography clean, correctly spelled, sharply legible at Full HD. Comfortable margins around the title and subtitle, natural proportions for every object and every letter. Art fills the entire 1920x1080 canvas, no border, no frame, no interface buttons, no extra lettering. Recreate the established atmospheric stone-wall/torch/dragon-shadow/blue-pool visual direction as a fresh composition for 1920x1080.

## Shared text-free background prompt

Remove ALL lettering from this image and reconstruct the stone wall naturally behind every letter. Remove the large OpenGoldBox title and the full two-line subtitle. Preserve the entire rest of this background composition: the torch and banner on the left, stone walls, dragon shadow, arched doorway at right, water and rocks, skull and props, warm/cool lighting. Output ONE text-free static background for two game splash screens that will share this exact same file. Do not add anything new, especially no text, lettering, logos, symbols or UI elements. Keep the natural proportions and wide landscape composition. Target canvas 1920x1080 if supported; do not warp the scene.

## Generator outputs

- Fresh composition: `exec-52862522-513a-404d-b0f1-72f065da9186.png`.
- Final shared background: `exec-eee56565-7e71-414c-85ef-33195362b517.png`,
  copied unchanged to `art/OpenGoldBoxSplashBackground.png`.
- Both outputs were saved under the built-in tool session
  `01a0a759-0ed9-7300-a3ee-61d2e1fa4d28`.
