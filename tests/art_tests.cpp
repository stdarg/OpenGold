#include "opengold/character_art.h"
#include "support/synthetic_dax.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace opengold;
using namespace opengold::por;
using namespace opengold::test;
namespace {
using Bytes = std::vector<std::uint8_t>;
void check(bool condition, const char* message)
{
    if (!condition) throw std::logic_error(message);
}
template<class F> void rejects(F&& operation, std::string_view diagnostic)
{
    try { operation(); }
    catch (const std::runtime_error& error) {
        check(std::string_view(error.what()).find(diagnostic) != std::string_view::npos,
              "Unexpected art rejection diagnostic");
        return;
    }
    throw std::logic_error("Invalid art was accepted");
}
Bytes picture(unsigned width, unsigned height, unsigned offset = 0)
{
    Bytes bytes(17+width*height/2);
    word(bytes,0,height); word(bytes,2,width/8); bytes[8] = 1;
    for (unsigned p = 0; p < width*height; p += 2) {
        const auto color = [&](unsigned pixel) {
            // Transparent holes make the tests exercise both body visibility
            // and head occlusion. Different offsets distinguish sizes/poses.
            return pixel%3 == 0 ? 0u : (pixel+offset)%16;
        };
        bytes[17+p/2] = (color(p)<<4)|color(p+1);
    }
    return bytes;
}
std::vector<DaxRecord> components(unsigned count, unsigned height)
{
    std::vector<DaxRecord> records;
    for (unsigned size : {0u,64u}) for (unsigned pose : {0u,128u})
        for (unsigned id = 0; id < count; ++id)
            records.push_back({static_cast<std::uint8_t>(size+pose+id),
                               picture(24,height,id+size/64+pose/32)});
    return records;
}
class Fixture {
public:
    Fixture() : path_(std::filesystem::temp_directory_path() /
        ("opengold-art-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())))
    {
        check(std::filesystem::create_directory(path_),"Unique art fixture directory");
    }
    ~Fixture() { std::error_code ignored; std::filesystem::remove_all(path_,ignored); }
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;
    const std::filesystem::path& path() const { return path_; }
    void write(std::string_view name, const Bytes& bytes) const
    {
        std::ofstream output(path_/name,std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());
        if (!output) throw std::runtime_error("Cannot write art fixture");
    }
    void populate() const
    {
        for (unsigned disk = 1; disk <= 8; ++disk) {
            write("HEAD"+std::to_string(disk)+".DAX",literal_dax({{1,picture(88,40)}}));
            write("BODY"+std::to_string(disk)+".DAX",literal_dax({{1,picture(88,48)}}));
        }
        write("CHEAD.DAX",literal_dax(components(14,8)));
        write("CBODY.DAX",literal_dax(components(32,24)));
    }
private:
    std::filesystem::path path_;
};

CharacterArt archive_tests()
{
    Fixture fixture;
    rejects([&] { (void)CharacterArt::load(fixture.path()); },"Missing character art");
    fixture.populate();
    auto art = CharacterArt::load(fixture.path());
    check(art.heads.size() == 1 && art.bodies.size() == 1,"Identical portrait IDs across disks merge");
    check(art.combat_heads.size() == 56 && art.combat_bodies.size() == 132,"Original and derived bodies load in all sizes and poses");
    std::filesystem::rename(fixture.path()/"CHEAD.DAX",fixture.path()/"chead.dax");
    check(CharacterArt::load(fixture.path()).combat_heads.size() == 56,"DOS filenames are case independent");
    std::filesystem::rename(fixture.path()/"chead.dax",fixture.path()/"CHEAD.DAX");

    const auto invalid_archive = [&](const char* filename, const Bytes& replacement, const char* message) {
        fixture.write(filename,replacement);
        rejects([&] { (void)CharacterArt::load(fixture.path()); },message);
        fixture.populate();
    };
    invalid_archive("HEAD8.DAX",literal_dax({{1,picture(88,40,1)}}),"Conflicting player portrait ID");
    invalid_archive("HEAD8.DAX",literal_dax({{2,picture(24,8)}}),"Unsupported player portrait layout");
    invalid_archive("HEAD8.DAX",Bytes{9,0},"Invalid character art archive");
    auto heads = components(14,8); heads.pop_back();
    invalid_archive("CHEAD.DAX",literal_dax(heads),"Missing combat head pose");
    auto bodies = components(32,24); bodies.erase(bodies.begin());
    invalid_archive("CBODY.DAX",literal_dax(bodies),"Missing combat body pose");
    invalid_archive("CBODY.DAX",literal_dax({{0,Bytes(16)}}),"Truncated character icon");
    for (unsigned disk = 1; disk <= 8; ++disk)
        fixture.write("HEAD"+std::to_string(disk)+".DAX",literal_dax({}));
    rejects([&] { (void)CharacterArt::load(fixture.path()); },"Missing portraits");
    fixture.populate();
    std::filesystem::resize_file(fixture.path()/"HEAD1.DAX",32*1024*1024+1);
    rejects([&] { (void)CharacterArt::load(fixture.path()); },"exceeds size limit");
    return art; // Its files disappear here; decoded art must own its data.
}

void composition_tests(const CharacterArt& art)
{
    CharacterAppearance appearance;
    check(art.portrait(appearance).rgba.size() == 88*88*4,"Portrait survives fixture destruction");
    for (bool tall : {false,true}) for (unsigned head = 0; head < 14; ++head)
        for (unsigned body = 0; body < 33; ++body) for (bool action : {false,true}) {
            appearance.tall = tall; appearance.combat_head = head; appearance.combat_body = body;
            const auto icon = art.icon(appearance,action);
            check(icon.width == 24 && icon.height == 24 && icon.rgba.size() == 576*4,
                  "Every synthetic head/body/size/pose composes");
        }

    constexpr std::array<unsigned,6> masks{7,1,4,6,2,3};
    for (bool tall : {false,true}) {
        appearance = {}; appearance.tall = tall;
        const auto usage = art.color_usage(appearance);
        for (bool action : {false,true}) {
            const unsigned source_bank = (tall ? 64 : 0)+(action ? 128 : 0);
            const auto& head = art.combat_heads.at(source_bank);
            const auto& body = art.combat_bodies.at(source_bank);
            const auto before = art.icon(appearance,action);
            for (unsigned bank = 0; bank < 2; ++bank) for (unsigned part = 0; part < 6; ++part) {
                unsigned targeted = 0;
                for (unsigned color = 0; color < 16; ++color) {
                    auto changed = appearance; changed.colors[bank][part] = color;
                    const auto after = art.icon(changed,action);
                    targeted = 0;
                    for (unsigned p = 0; p < 576; ++p) {
                        const auto source = p < head.pixels.size() && head.pixels[p] ? head.pixels[p] : body.pixels[p];
                        const bool selected = source == masks[part]+8*bank;
                        const bool differs = !std::equal(before.rgba.begin()+4*p,before.rgba.begin()+4*p+4,after.rgba.begin()+4*p);
                        check(differs == (selected && color != appearance.colors[bank][part]),
                              "Recolor changes exactly the visible selected region, preserving transparency and fixed colors");
                        targeted += selected;
                    }
                }
                check(targeted == (action ? usage.action : usage.ready)[bank][part],
                      "Usage count equals the pixels recolored in each pose");
                check(usage.contains(bank,part) == (usage.ready[bank][part]+usage.action[bank][part] > 0),
                      "A control is relevant if either pose uses its region");
            }
            check(art.icon(appearance,action).rgba == before.rgba,"Recoloring never mutates cached components");
        }
    }
    rejects([&] { (void)art.color_usage(appearance).contains(2,0); },"Invalid character color region");
    rejects([&] { (void)art.color_usage(appearance).contains(0,6); },"Invalid character color region");
    appearance.portrait_head = 2;
    rejects([&] { (void)art.portrait(appearance); },"Invalid character appearance selection");
    auto head = art.combat_heads.at(0), body = art.combat_bodies.at(0);
    head.pixels[0] = 16;
    rejects([&] { (void)compose_character_icon(head,body,{}); },"Invalid character icon pixel");
    head.pixels.pop_back();
    rejects([&] { (void)compose_character_icon(head,body,{}); },"Invalid character icon components");
}
}
int main()
{
    try {
        composition_tests(archive_tests());
        std::cout << "Synthetic art loading and recolor properties passed\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
