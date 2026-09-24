#include "character_creation_view.h"
#include "localization.h"
namespace { godot::String review_text(std::string_view value){return i18n::text(value);} }
#include "training_review_impl.h"
