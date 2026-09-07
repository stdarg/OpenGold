#include "opengold/character_art.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace opengold::por {
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
    if(head.width!=24 || body.width!=24 || !head.height || head.height>24 || body.height!=24 ||
        head.pixels.size()!=head.width*head.height || body.pixels.size()!=576)
        throw std::runtime_error("Invalid character icon components");
    for(const auto& bank:appearance.colors)for(auto c:bank)(void)character_color(c);
    Image result;result.width=result.height=24;result.rgba.resize(24*24*4);
    // Source indices 1/9 = body, 2/10 = arms, 3/11 = legs,
    // 4/12 = hair/face, 5/13 = shield, 6/14 = weapon.
    constexpr std::array<unsigned,7> region{0,1,4,5,2,3,0};
    for(unsigned p=0;p<576;++p) {
        unsigned index=body.pixels[p];
        if(p<head.pixels.size() && head.pixels[p])index=head.pixels[p];
        if(index>15)throw std::runtime_error("Invalid character icon pixel");
        auto rgb=character_color(index==8?0:index);
        if(index && index!=8 && (index&7)<=6)
            rgb=character_color(appearance.colors[index>>3][region[index&7]]);
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
    if(!heads.contains(a.portrait_head)||!bodies.contains(a.portrait_body)||a.combat_head>=14||a.combat_body>=32)
        throw std::runtime_error("Invalid character appearance selection");
    for(const auto& bank:a.colors)for(auto color:bank)(void)character_color(color);
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
}
