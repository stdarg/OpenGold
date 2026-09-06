#include "opengold/core.h"
#include "opengold/formats.h"

#include <array>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

int main()
{
    const std::array<std::uint8_t, 4> bytes{0x4f, 0x50, 0x47, 0x44};
    const opengold::Core core;

    assert(core.checksum(bytes) == 0xcb4b7229u);

    // One 2x1-byte (16x1 pixel) synthetic EGA sprite frame in a DAX record.
    const std::array<std::uint8_t, 42> dax{
        9, 0, 2, 0, 0, 0, 0, 30, 0, 31, 0,
        29, 1, 0, 0, 0, 0, 1, 0, 2, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0,
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    const auto decoded = opengold::decode_ega_sprite(dax, 2);
    assert(decoded);
    assert(decoded.image.width == 16 && decoded.image.height == 1 && decoded.image.rgba.size() == 64);
    assert(decoded.image.rgba[3] == 255 && decoded.image.rgba[63] == 0);

    if (const char *game_dir = std::getenv("OPENGOLD_GAME_DIR")) {
        std::ifstream input(std::filesystem::path(game_dir) / "SPRIT1.DAX", std::ios::binary);
        const std::vector<std::uint8_t> installed{
            std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        const auto installed_image = opengold::decode_ega_sprite(installed, 2);
        assert(installed_image);
        assert(installed_image.image.width == 48 && installed_image.image.height == 80);
    }
    return 0;
}
