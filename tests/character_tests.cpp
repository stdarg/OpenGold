#include "opengold/character_art.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

using namespace opengold;
using namespace opengold::por;
namespace {
void check(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
template<class F> void rejects(F action,const char* message)
{bool rejected=false;try{action();}catch(const std::exception&){rejected=true;}check(rejected,message);}
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
    try {art_tests();std::cout<<"Character tests passed\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
