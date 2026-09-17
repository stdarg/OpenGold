#!/usr/bin/env bash
set -euo pipefail
demo_directory="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
godot_binary="${GODOT_BIN:-}"
if [[ -z "$godot_binary" ]]; then
    godot_binary="$(command -v godot || command -v godot4 || true)"
fi
if [[ -z "$godot_binary" && -x /Applications/Godot_mono.app/Contents/MacOS/Godot ]]; then
    godot_binary=/Applications/Godot_mono.app/Contents/MacOS/Godot
fi
if [[ -z "$godot_binary" && -x /Applications/Godot.app/Contents/MacOS/Godot ]]; then
    godot_binary=/Applications/Godot.app/Contents/MacOS/Godot
fi
if [[ -z "$godot_binary" ]]; then
    echo "Godot was not found. Set GODOT_BIN to its executable." >&2
    exit 1
fi
if [[ ! -f "$demo_directory/godot/bin/opengold.gdextension" ]]; then
    echo "Build the demo extension first; see docs/SPRITE-DEMO.md." >&2
    exit 1
fi
"$godot_binary" --headless --editor --path "$demo_directory/godot" --import --quit
exec "$godot_binary" --path "$demo_directory/godot" --resolution 1920x1080 res://scenes/combat_sprite_demo.tscn "$@"
