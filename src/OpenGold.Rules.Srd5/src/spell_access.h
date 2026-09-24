#ifndef OPENGOLD_SRD5_SPELL_ACCESS_H
#define OPENGOLD_SRD5_SPELL_ACCESS_H
#include "opengold/character_rules.h"
namespace opengold::srd5::detail {
bool is_spell_grant(const rules::FeatureGrant&);
std::vector<rules::FeatureGrant> without_spell_grants(std::span<const rules::FeatureGrant>);
std::vector<rules::FeatureGrant> starting_spell_grants(std::string_view klass,
    const std::optional<std::vector<std::string>>& cantrips={});
rules::TrainingChoiceGroup starting_cantrip_options(std::string_view klass);
rules::SpellAccess spell_access(std::span<const rules::FeatureGrant>,std::string_view klass,unsigned level,
    std::span<const std::string> prepared);
// Existing advancement selections learn any newly selected book spell and
// retain every earlier entry. Copying and preparation controls are separate.
void learn_advancement_spells(rules::CharacterSheet&,std::span<const std::string> selected);
unsigned wizard_casting_mask(const rules::SpellAccess&);
}
#endif
