shader_type canvas_item;

/*
* Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
*
* Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net )
*
* Copyright (C) 2014 Jules Blok ( jules.blok@gmail.com )
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
render_mode unshaded;

// Godot adaptation of CrossVR's HQ4x shader; see README.md and COPYING.
// Input is opaque, background-composited art. LUT values are data, not sRGB.
uniform sampler2D lookup_table : filter_nearest, repeat_disable;

const vec3 YUV_THRESHOLD = vec3(48.0 / 255.0, 7.0 / 255.0, 6.0 / 255.0);

vec3 to_yuv(vec3 rgb) {
    return vec3(dot(rgb, vec3(0.299, 0.587, 0.114)),
                dot(rgb, vec3(-0.169, -0.331, 0.5)),
                dot(rgb, vec3(0.5, -0.419, -0.081)));
}

bool different(vec3 a, vec3 b) {
    return any(greaterThan(abs(a - b), YUV_THRESHOLD));
}

void fragment() {
    vec2 fp = fract(UV / TEXTURE_PIXEL_SIZE);
    vec2 quadrant = sign(fp - vec2(0.5));
    vec2 ps = TEXTURE_PIXEL_SIZE;
    vec3 p1 = texture(TEXTURE, UV).rgb;
    vec3 p2 = texture(TEXTURE, UV + ps * quadrant).rgb;
    vec3 p3 = texture(TEXTURE, UV + vec2(ps.x, 0.0) * quadrant).rgb;
    vec3 p4 = texture(TEXTURE, UV + vec2(0.0, ps.y) * quadrant).rgb;

    vec3 w1 = to_yuv(texture(TEXTURE, UV + ps * vec2(-1.0, -1.0)).rgb);
    vec3 w2 = to_yuv(texture(TEXTURE, UV + ps * vec2(0.0, -1.0)).rgb);
    vec3 w3 = to_yuv(texture(TEXTURE, UV + ps * vec2(1.0, -1.0)).rgb);
    vec3 w4 = to_yuv(texture(TEXTURE, UV + ps * vec2(-1.0, 0.0)).rgb);
    vec3 w5 = to_yuv(p1);
    vec3 w6 = to_yuv(texture(TEXTURE, UV + ps * vec2(1.0, 0.0)).rgb);
    vec3 w7 = to_yuv(texture(TEXTURE, UV + ps * vec2(-1.0, 1.0)).rgb);
    vec3 w8 = to_yuv(texture(TEXTURE, UV + ps * vec2(0.0, 1.0)).rgb);
    vec3 w9 = to_yuv(texture(TEXTURE, UV + ps * vec2(1.0, 1.0)).rgb);

    float pattern = dot(vec3(bvec3(different(w5, w1), different(w5, w2), different(w5, w3))), vec3(1.0, 2.0, 4.0))
        + dot(vec2(bvec2(different(w5, w4), different(w5, w6))), vec2(8.0, 16.0))
        + dot(vec3(bvec3(different(w5, w7), different(w5, w8), different(w5, w9))), vec3(32.0, 64.0, 128.0));
    float cross_pattern = dot(vec4(bvec4(different(w4, w2), different(w2, w6), different(w8, w4), different(w6, w8))), vec4(1.0, 2.0, 4.0, 8.0));
    vec2 index = vec2(pattern, cross_pattern * 16.0 + dot(floor(fp * 4.0), vec2(1.0, 4.0)));
    vec4 weights = texture(lookup_table, (index + vec2(0.5)) / vec2(256.0));
    weights /= dot(weights, vec4(1.0));
    COLOR = vec4(p1 * weights.x + p2 * weights.y + p3 * weights.z + p4 * weights.w, 1.0);
}
