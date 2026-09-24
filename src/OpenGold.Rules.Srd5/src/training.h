#ifndef OPENGOLD_SRD5_TRAINING_H
#define OPENGOLD_SRD5_TRAINING_H
#include "opengold/character_rules.h"
namespace opengold::srd5::detail {
// Profile versions retain the fixed-grant catalog available to their writer.
enum class TrainingPolicy { legacy, sage, all_backgrounds };
std::vector<rules::TrainingChoiceGroup> training_options(const rules::CharacterDraft& draft);
std::vector<rules::FeatureGrant> training_grants(std::string_view klass,std::string_view background,const rules::TrainingChoices& choices,TrainingPolicy policy=TrainingPolicy::all_backgrounds);
bool is_training_grant(const rules::FeatureGrant& grant);
std::vector<rules::FeatureGrant> without_training(std::span<const rules::FeatureGrant> grants);
rules::TrainingChoices training_choices(std::span<const rules::FeatureGrant> grants,std::string_view klass,std::string_view background,TrainingPolicy policy=TrainingPolicy::all_backgrounds);
rules::TrainingProfile training_profile(std::span<const rules::FeatureGrant> grants,std::string_view klass,
    std::string_view background,unsigned level,const std::array<int,6>& scores,TrainingPolicy policy=TrainingPolicy::all_backgrounds);
rules::AbilityCheckModifier ability_check(std::span<const rules::FeatureGrant> grants,std::string_view klass,
    std::string_view background,unsigned level,const std::array<int,6>& scores,unsigned ability,
    std::string_view skill,std::string_view tool);
}
#endif
