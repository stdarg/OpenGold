#ifndef OPENGOLD_TEST_COMBAT_FIXTURE_H
#define OPENGOLD_TEST_COMBAT_FIXTURE_H
#include <cstdint>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace opengold::test {
// Independent expected transformation for frozen format-eight files: replace
// identity/format and append one known Hit Dice count to each actor row. Every
// other byte (including recipes, RNG, turn state, effects and queues) is retained.
inline std::string with_hit_dice(std::string_view bytes,std::string_view version,const std::map<unsigned,unsigned>& counts)
{
    std::istringstream input{std::string(bytes)};std::vector<std::string> rows;
    for(std::string row;std::getline(input,row);)rows.push_back(std::move(row));
    if(rows.size()<4)throw std::runtime_error("Incomplete frozen combat");
    std::istringstream header(rows[0]);std::string magic,module,previous,content;unsigned format{};
    header>>magic>>format>>std::quoted(module)>>std::quoted(previous)>>std::quoted(content);
    if(!header||magic!="OGCOMBAT"||format!=8)throw std::runtime_error("Expected frozen format eight");
    std::ostringstream next;next<<"OGCOMBAT 9 "<<std::quoted(module)<<' '<<std::quoted(std::string(version))<<' '<<std::quoted(content);rows[0]=next.str();
    std::istringstream state(rows[3]);std::uint64_t value{};for(unsigned i=0;i<5;++i)state>>value;
    unsigned size{};state>>size;
    if(!state||rows.size()<4+size||size!=counts.size())throw std::runtime_error("Unexpected frozen actor count");
    for(unsigned i=0;i<size;++i){std::istringstream actor(rows[4+i]);unsigned id{};actor>>id;rows[4+i]+=' '+std::to_string(counts.at(id));}
    std::string result;for(const auto& row:rows)result+=row+'\n';return result;
}
}
#endif
