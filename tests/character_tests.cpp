#include "opengold/character_art.h"
#include "opengold/character_creator.h"
#include "opengold/srd5.h"
#include <algorithm>
#include <set>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace opengold;
using namespace opengold::por;
namespace {
void check(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
template<class F> void rejects(F action,const char* message)
{bool rejected=false;try{action();}catch(const std::exception&){rejected=true;}check(rejected,message);}
void creation_tests()
{
    using namespace rules;
    auto module=srd5::character_rules();
    check(module->identity().version=="5.2.1","Creator identifies its rules edition");
    check(module->choices(CreationField::race).size()==9&&module->choices(CreationField::character_class).size()==12,
        "SRD species and all twelve classes available");
    std::uint64_t seed=123,again=123;
    check(module->roll(seed)==module->roll(again)&&seed==again,"Seeded rolls reproduce across sessions");
    std::set<int> totals;
    for(int i=0;i<2000;++i)for(const auto& roll:module->roll(seed)) {
        check(roll.total()>=3&&roll.total()<=18,"4d6 result remains in range");totals.insert(roll.total());
        check(roll.dice[roll.discarded]==*std::min_element(roll.dice.begin(),roll.dice.end()),"Lowest die is discarded");
    }
    check(totals.size()==16,"The roller can produce every score, including low results");
    AbilityRoll example{{6,1,4,3},1};check(example.total()==13,"Four dice retain the three highest results");
    example.discarded=0;rejects([&]{(void)example.total();},"Cannot discard a higher die");
    CharacterDraft d;d.race="human";d.gender="female";d.character_class="fighter";d.alignment="neutral_good";
    d.background="soldier";d.name="Mira";d.rolled=true;
    // Deliberately distinct base scores: STR 12, DEX 9, CON 14, INT 3, WIS 15, CHA 18.
    const std::array<std::array<int,4>,6> dice{{{4,4,4,1},{3,3,3,1},{5,5,4,1},{1,1,1,1},{5,5,5,1},{6,6,6,1}}};
    for(unsigned i=0;i<6;++i)d.rolls[i]={dice[i],3};
    auto s=module->evaluate(d,true);
    check(s.scores[0]==14&&s.scores[1]==10&&s.scores[2]==14&&s.hit_points==12,"Fighter has maximum d10 plus Constitution");
    check(s.modifiers[3]==-4,"Odd negative ability modifiers round down");
    const std::array<int,12> hp{14,10,10,10,12,10,12,12,10,8,10,8};
    const auto classes=module->choices(CreationField::character_class);
    for(unsigned i=0;i<classes.size();++i){d.character_class=classes[i].id;check(module->evaluate(d,true).hit_points==hp[i],"Starting HP is correct for each class");}
    d.character_class="fighter";d.race="dwarf";check(module->evaluate(d,true).hit_points==13,"Dwarven Toughness adds one HP");
    for(const auto& bg:module->choices(CreationField::background)) {
        d.background=bg.id;const auto adjustments=module->adjustments(bg.id);check(adjustments.size()==7,"Every background offers six +2/+1 allocations and +1 each");
        for(unsigned n=0;n<adjustments.size();++n) {d.adjustment=n;const auto evaluated=module->evaluate(d,true);
            int bonus=0;for(unsigned k=0;k<6;++k){bonus+=evaluated.bonuses[k];check(evaluated.scores[k]<=20,"Background bonuses cap at twenty");}
            check(bonus==3,"Background grants exactly three points");}
    }
    d.background="soldier";d.adjustment=0;d.assignment[0]=1;
    rejects([&]{(void)module->evaluate(d,true);},"Duplicate roll assignments rejected");d.assignment[0]=0;
    d.name="  ";rejects([&]{(void)module->evaluate(d,true);},"Blank names rejected");
    d.name="Mira";d.character_class="invented";rejects([&]{(void)module->evaluate(d,true);},"Unknown class rejected");
    CharacterCreator creator(srd5::character_rules(),42);
    for(int i=0;i<4;++i)creator.next();
    rejects([&]{creator.next();},"Cannot advance without rolling");check(creator.step()==CreationStep::attributes,"Invalid transition leaves step unchanged");
    creator.roll();const auto original=creator.draft().rolls;creator.swap_scores(0,2);
    check(creator.sheet().base[0]==original[2].total()&&creator.draft().rolls==original,"Swaps preserve dice provenance");
    const auto assignment=creator.draft().assignment;
    rejects([&]{creator.swap_scores(0,6);},"Invalid swap rejected");check(creator.draft().assignment==assignment,"Rejected swap is atomic");
    creator.roll();check(creator.draft().rolls!=original&&creator.draft().assignment[0]==0,"Full reroll replaces all rolls and resets assignments");
    creator.next();creator.next();rejects([&]{creator.next();},"Name required before portrait");
    creator.name("  Mira Stoneward  ");creator.next();creator.next();
    auto appearance=creator.appearance();appearance.combat_head=9;appearance.colors[1][5]=0;creator.appearance(appearance);creator.next();
    check(creator.step()==CreationStep::sheet&&creator.sheet().name=="Mira Stoneward","Completed sheet retains trimmed name");
    rejects([&]{creator.roll();},"Completed sheet cannot be silently rerolled");
    creator.back();creator.next();check(creator.appearance()==appearance,"Review retains both appearance banks");
    creator.back();creator.back();creator.back();creator.back();creator.back();
    check(creator.step()==CreationStep::attributes,"Back navigation reaches attributes");
    creator.select(CreationField::race,"dwarf");creator.select(CreationField::character_class,"barbarian");
    check(creator.sheet().hit_points==13+creator.sheet().modifiers[2],"HP recalculates after earlier edits");
    creator.restart();check(!creator.draft().rolled&&creator.draft().name.empty()&&creator.step()==CreationStep::race,"Restart clears the single character");
}
void art_tests()
{
    std::vector<std::uint8_t> raw(17+24*24/2);raw[0]=24;raw[2]=3;raw[8]=1;raw[17]=0x6e;
    auto body=decode_character_icon(raw);
    check(body.pixels[0]==6&&body.pixels[1]==14,"Preserve region indices and nibble order");
    raw.pop_back();rejects([&]{(void)decode_character_icon(raw);},"Reject truncated components");
    IndexedIcon head{24,10,std::vector<std::uint8_t>(240)};
    CharacterAppearance a;const std::array<unsigned,6> masks{6,1,4,5,2,3};
    for(unsigned bank=0;bank<2;++bank)for(unsigned region=0;region<6;++region) {
        body.pixels.assign(576,0);body.pixels[0]=masks[region]+bank*8;
        body.pixels[1]=8;body.pixels[2]=7;
        a.colors[bank][region]=0;const auto black=compose_character_icon(head,body,a);
        check(black.rgba[3]==255&&black.rgba[0]==0,"Chosen black must remain opaque");
        a.colors[bank][region]=14;const auto yellow=compose_character_icon(head,body,a);
        check(yellow.rgba[0]==255&&yellow.rgba[1]==255&&yellow.rgba[2]==85,"Each bank/region maps independently");
        check(yellow.rgba[4]==0&&yellow.rgba[7]==255&&yellow.rgba[15]==0,"Outline and transparency preserved");
        check(yellow.rgba[8]==black.rgba[8],"Recolor does not alter fixed cap pixels");
    }
    head.pixels[0]=4;a.colors[0][2]=10;body.pixels[0]=6;
    auto composed=compose_character_icon(head,body,a);
    check(composed.rgba[0]==85&&composed.rgba[1]==255,"Head overlays body using indexed colors");
    a.colors[1][5]=16;rejects([&]{(void)compose_character_icon(head,body,a);},"Reject invalid colors");
    if(const auto directory=std::getenv("OPENGOLD_GAME_DIR")) {
        const auto art=CharacterArt::load(directory);a=CharacterAppearance{};
        check(art.portrait(a).rgba.size()==88*88*4,"Original portrait parts join into a sheet image");
        for(bool tall:{false,true})for(unsigned h=0;h<14;++h)for(unsigned b=0;b<32;++b) {
            a.tall=tall;a.combat_head=h;a.combat_body=b;
            check(art.icon(a,false).rgba.size()==576*4&&art.icon(a,true).rgba.size()==576*4,
                "Every original head/body combination has both poses");
        }
        std::cout<<"Original character art: "<<art.heads.size()<<" portrait heads, "<<art.bodies.size()
            <<" bodies; 14 combat heads, 32 bodies, both sizes and poses\n";
    }
}
}
int main()
{
    try {creation_tests();art_tests();std::cout<<"Character tests passed\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
