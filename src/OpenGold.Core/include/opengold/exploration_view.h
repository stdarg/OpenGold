#ifndef OPENGOLD_EXPLORATION_VIEW_H
#define OPENGOLD_EXPLORATION_VIEW_H

#include "opengold/geo_map.h"
#include "opengold/wall_art.h"

namespace opengold::por {
// Original perspective pieces are composed on an 88x88 logical canvas. The UI
// controls scaling; this function never changes map, door or encounter state.
[[nodiscard]] Image compose_exploration_view(
    const GeoMap& map, const WallArtSet& art, unsigned x, unsigned y, unsigned facing);
}
#endif
