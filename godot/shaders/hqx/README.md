# HQ4x shader

HQx algorithm by Maxim Stepin; C implementation credits Maxim Stepin and
Cameron Zemek; GPU implementation by Jules Blok (CrossVR).

Source: https://github.com/CrossVR/hqx-shader
Pinned revision: 53540f5f0d985c385dc108b41ab89980f2b214f4
Upstream files: glsl/hq4x.glsl, resources/hq4x.png, COPYING.

The shader and lookup table are LGPL-2.1-or-later. See COPYING for the full
license. The adapted shader source is provided here under the same license;
it can be edited and replaced in the Godot project independently of game code.
Preserve these files and notices when distributing the project and provide the
corresponding shader source with distributions containing this component.

OpenGold modifications: ported to Godot canvas_item syntax, replaced vertex
varyings with fragment texture offsets, expanded YUV matrix and weighted pixel
multiplication into dot products/sums, and explicitly wrote opaque alpha.
The original HQ4x pattern bits, YUV thresholds and 256x256 lookup table are
preserved. The lookup PNG is unmodified and must be sampled as linear data
with nearest filtering, lossless import, and no mipmaps.

This is HQ4x at a fixed 4x scale, not two successive HQ2x passes. Like the xBR
comparison, it filters an image composited onto the demo background first.
