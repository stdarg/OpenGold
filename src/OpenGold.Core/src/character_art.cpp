#include "opengold/character_art.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace opengold::por {
namespace {
constexpr std::array<AdditionalPortraitHead,10> additional_heads{{
    {256,"gnome-male.png","Gnome / Male","gnome","male"},
    {257,"gnome-female.png","Gnome / Female","gnome","female"},
    {258,"orc-male.png","Orc / Male","orc","male"},
    {259,"orc-female.png","Orc / Female","orc","female"},
    {260,"goliath-male.png","Goliath / Male","goliath","male"},
    {261,"goliath-female.png","Goliath / Female","goliath","female"},
    {262,"tiefling-male.png","Tiefling / Male","tiefling","male"},
    {263,"tiefling-female.png","Tiefling / Female","tiefling","female"},
    {264,"dragonborn-male.png","Dragonborn / Male","dragonborn","male"},
    {265,"dragonborn-female.png","Dragonborn / Female","dragonborn","female"}
}};
// 5/13 belong to the cap. The six user-customizable regions skip that pair.
constexpr std::array<int,8> color_regions{-1,1,4,5,2,-1,3,0};
std::vector<std::uint8_t> composed_pixels(const IndexedIcon& head,const IndexedIcon& body)
{
    if(head.width!=24 || body.width!=24 || !head.height || head.height>24 || body.height!=24 ||
        head.pixels.size()!=head.width*head.height || body.pixels.size()!=576)
        throw std::runtime_error("Invalid character icon components");
    auto pixels=body.pixels;
    for(unsigned p=0;p<head.pixels.size();++p)if(head.pixels[p])pixels[p]=head.pixels[p];
    if(std::any_of(pixels.begin(),pixels.end(),[](auto c){return c>15;}))
        throw std::runtime_error("Invalid character icon pixel");
    return pixels;
}
}
std::span<const AdditionalPortraitHead> additional_portrait_heads() {return additional_heads;}
std::optional<unsigned> matching_portrait_head(std::string_view race,std::string_view gender)
{
    for(const auto& head:additional_heads)if(head.race==race&&head.gender==gender)return head.id;
    return std::nullopt;
}
Image prepare_portrait_head(const Image& source)
{
    if(!source.width||!source.height||source.width>8192||source.height>8192||
        source.rgba.size()!=std::size_t(source.width)*source.height*4)
        throw std::runtime_error("Invalid portrait source image");
    unsigned bottom=source.height;
    const auto visible=[&](unsigned x,unsigned y) {
        const auto p=(std::size_t(y)*source.width+x)*4;
        return source.rgba[p+3]>127&&std::max({source.rgba[p],source.rgba[p+1],source.rgba[p+2]})>24;
    };
    while(bottom) {
        bool content=false;
        for(unsigned x=0;x<source.width;++x)if(visible(x,bottom-1)){content=true;break;}
        if(content)break;
        --bottom;
    }
    if(!bottom)throw std::runtime_error("Portrait source has no visible head");
    Image result;result.width=88;result.height=40;result.rgba.resize(88*40*4);
    for(unsigned y=0;y<40;++y)for(unsigned x=0;x<88;++x) {
        const auto from=(std::size_t((2*y+1)*bottom/80)*source.width+(2*x+1)*source.width/176)*4;
        const auto to=(y*88+x)*4;
        // Composite alpha on the original portrait's black background.
        for(unsigned c=0;c<3;++c)result.rgba[to+c]=unsigned(source.rgba[from+c])*source.rgba[from+3]/255;
        result.rgba[to+3]=255;
    }
    return result;
}
bool CharacterColorUsage::contains(unsigned bank,unsigned part) const
{
    if(bank>=2||part>=6)throw std::runtime_error("Invalid character color region");
    return ready[bank][part]||action[bank][part];
}
void validate_character_appearance(const CharacterAppearance& a)
{
    if((a.portrait_head>255&&std::none_of(additional_heads.begin(),additional_heads.end(),[&](const auto& h){return h.id==a.portrait_head;}))||
        a.portrait_body>255||a.combat_head>=14||a.combat_body>=32)
        throw std::runtime_error("Invalid appearance reference");
    for(const auto& bank:a.colors)for(auto color:bank)(void)character_color(color);
}
std::array<std::uint8_t, 3> character_color(unsigned index)
{
    constexpr std::array<std::array<std::uint8_t, 3>, 16> palette{{
        {0,0,0},{0,0,170},{0,170,0},{0,170,170},{170,0,0},{170,0,170},{170,85,0},{170,170,170},
        {85,85,85},{85,85,255},{85,255,85},{85,255,255},{255,85,85},{255,85,255},{255,255,85},{255,255,255}}};
    if(index>=palette.size())throw std::runtime_error("Invalid character color");
    return palette[index];
}
IndexedIcon decode_character_icon(std::span<const std::uint8_t> record)
{
    if(record.size()<17)throw std::runtime_error("Truncated character icon");
    const unsigned height=record[0]+256u*record[1], width=(record[2]+256u*record[3])*8;
    if(width!=24 || !height || height>24 || record[8]!=1 || record.size()!=17+width*height/2)
        throw std::runtime_error("Unsupported character icon layout");
    IndexedIcon result{width,height,{}};result.pixels.reserve(width*height);
    for(unsigned p=0;p<width*height;++p)
        result.pixels.push_back(p%2?record[17+p/2]&15:record[17+p/2]>>4);
    return result;
}
Image compose_character_icon(const IndexedIcon& head,const IndexedIcon& body,const CharacterAppearance& appearance)
{
    const auto pixels=composed_pixels(head,body);
    for(const auto& bank:appearance.colors)for(auto c:bank)(void)character_color(c);
    Image result;result.width=result.height=24;result.rgba.resize(24*24*4);
    // Source indices 1/9 = body, 2/10 = arms, 3/11 = legs,
    // 4/12 = hair/face, 6/14 = shield, 7/15 = weapon.
    for(unsigned p=0;p<576;++p) {
        const unsigned index=pixels[p];
        auto rgb=character_color(index==8?0:index);
        if(const auto region=color_regions[index&7];region>=0)
            rgb=character_color(appearance.colors[index>>3][region]);
        std::copy(rgb.begin(),rgb.end(),result.rgba.begin()+p*4);
        result.rgba[p*4+3]=index?255:0;
    }
    return result;
}
CharacterArt CharacterArt::load(const std::filesystem::path& directory)
{
    std::map<std::string,std::filesystem::path> paths;
    for(const auto& entry:std::filesystem::directory_iterator(directory))if(entry.is_regular_file()) {
        auto name=entry.path().filename().string();
        for(auto& c:name)if(c>='a'&&c<='z')c-=32;
        if(!paths.emplace(name,entry.path()).second)throw std::runtime_error("Ambiguous art archive: "+name);
    }
    const auto read=[&](const std::string& name) {
        if(!paths.contains(name))throw std::runtime_error("Missing character art: "+name);
        const auto& path=paths.at(name);const auto size=std::filesystem::file_size(path);
        if(size>32*1024*1024)throw std::runtime_error("Character art archive exceeds size limit");
        std::ifstream in(path,std::ios::binary);
        std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(in),{}};
        if(in.bad()||bytes.size()!=size)throw std::runtime_error("Cannot read character art: "+name);
        auto decoded=decode_dax_archive(bytes);
        if(!decoded)throw std::runtime_error("Invalid character art archive: "+name);
        return decoded.records;
    };
    CharacterArt art;
    for(unsigned disk=1;disk<=8;++disk)for(const auto& stem:{std::string("HEAD"),std::string("BODY")}) {
        const auto archive=stem+std::to_string(disk)+".DAX";
        for(const auto& r:read(archive)) {
            // Present the available original parts. Do not interpret a creator
            // menu index as an archive ID, or silently invent missing images.
            auto decoded=decode_ega_picture(r.bytes);
            if(!decoded || decoded.image.width!=88 || decoded.image.height!=(stem=="HEAD"?40:48))
                throw std::runtime_error("Unsupported player portrait layout");
            auto& parts=stem=="HEAD"?art.heads:art.bodies;
            if(parts.contains(r.id) && parts.at(r.id).image.rgba!=decoded.image.rgba)
                throw std::runtime_error("Conflicting player portrait ID across archives");
            parts.try_emplace(r.id,PortraitPart{archive,std::move(decoded.image)});
        }
    }
    for(const auto& r:read("CHEAD.DAX"))art.combat_heads.emplace(r.id,decode_character_icon(r.bytes));
    for(const auto& r:read("CBODY.DAX"))art.combat_bodies.emplace(r.id,decode_character_icon(r.bytes));
    if(art.heads.empty() || art.bodies.empty())throw std::runtime_error("Missing portraits");
    for(unsigned size:{0u,64u})for(unsigned pose:{0u,128u}) {
        for(unsigned id=0;id<14;++id)if(!art.combat_heads.contains(size+pose+id))throw std::runtime_error("Missing combat head pose");
        for(unsigned id=0;id<32;++id)if(!art.combat_bodies.contains(size+pose+id))throw std::runtime_error("Missing combat body pose");
    }
    return art;
}
void CharacterArt::validate(const CharacterAppearance& a) const
{
    validate_character_appearance(a);
    if(!heads.contains(a.portrait_head)||!bodies.contains(a.portrait_body))
        throw std::runtime_error("Invalid character appearance selection");
}
void CharacterArt::add_portrait_head(unsigned id,Image image)
{
    const auto entry=std::find_if(additional_heads.begin(),additional_heads.end(),[&](const auto& head){return head.id==id;});
    if(entry==additional_heads.end()||image.width!=88||image.height!=40||image.rgba.size()!=88*40*4)
        throw std::runtime_error("Invalid additional portrait head");
    if(!heads.emplace(id,PortraitPart{std::string(entry->filename),std::move(image),std::string(entry->label)}).second)
        throw std::runtime_error("Duplicate portrait head ID");
}
Image CharacterArt::portrait(const CharacterAppearance& a) const
{
    validate(a);Image result;result.width=result.height=88;
    result.rgba=heads.at(a.portrait_head).image.rgba;
    const auto& body=bodies.at(a.portrait_body).image.rgba;
    result.rgba.insert(result.rgba.end(),body.begin(),body.end());return result;
}
Image CharacterArt::icon(const CharacterAppearance& a,bool action) const
{
    validate(a);const unsigned bank=(a.tall?64u:0u)+(action?128u:0u);
    return compose_character_icon(combat_heads.at(bank+a.combat_head),combat_bodies.at(bank+a.combat_body),a);
}
CharacterColorUsage CharacterArt::color_usage(const CharacterAppearance& a) const
{
    validate(a);CharacterColorUsage usage;
    for(bool action:{false,true}) {
        const unsigned bank=(a.tall?64u:0u)+(action?128u:0u);
        const auto pixels=composed_pixels(combat_heads.at(bank+a.combat_head),combat_bodies.at(bank+a.combat_body));
        auto& counts=action?usage.action:usage.ready;
        for(auto pixel:pixels)if(const auto part=color_regions[pixel&7];part>=0)++counts[pixel>>3][part];
    }
    return usage;
}
}
