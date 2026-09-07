#ifndef OPENGOLD_SPEAKER_AUDIO_H
#define OPENGOLD_SPEAKER_AUDIO_H
#include "opengold/por_sound.h"

namespace opengold::por {
inline constexpr unsigned speaker_sample_rate=48000;
// Mono signed PCM; playback gain/mute belong to the presentation layer.
[[nodiscard]] std::vector<std::int16_t> render_speaker_audio(const SoundEffect& effect);
}
#endif
