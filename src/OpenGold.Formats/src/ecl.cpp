#include "opengold/ecl.h"
#include <utility>

namespace opengold::por {
const EclOpcode& ecl_opcode(std::uint8_t op)
{
    // PoR grammar matches the existing inspector, including the one-operand clock.
    static const std::array<EclOpcode, 62> specs{{
        {"EXIT",0,false,true}, {"GOTO",1,false,true}, {"GOSUB",1,false,true}, {"COMPARE",2,false,true},
        {"ADD",3,false,true}, {"SUBTRACT",3,false,true}, {"DIVIDE",3}, {"MULTIPLY",3,false,true},
        {"RANDOM",2}, {"SAVE",2,false,true}, {"LOAD CHARACTER",1}, {"LOAD MONSTER",3},
        {"SETUP MONSTER",3}, {"APPROACH",0}, {"PICTURE",1}, {"INPUT NUMBER",2},
        {"INPUT STRING",2}, {"PRINT",1,false,true}, {"PRINTCLEAR",1,false,true}, {"RETURN",0,false,true},
        {"COMPARE AND",4}, {"VERTICAL MENU",3,true,true}, {"IF =",0,false,true}, {"IF <>",0,false,true},
        {"IF <",0,false,true}, {"IF >",0,false,true}, {"IF <=",0,false,true}, {"IF >=",0,false,true},
        {"CLEAR MONSTERS",0}, {"PARTY STRENGTH",1}, {"CHECK PARTY",6}, {"UNKNOWN 1F",2},
        {"NEW ECL",1}, {"LOAD FILES",3}, {"PARTY SURPRISE",2}, {"SURPRISE",4},
        {"COMBAT",0}, {"ON GOTO",2,true,true}, {"ON GOSUB",2,true,true}, {"TREASURE",8},
        {"ROB",3}, {"ENCOUNTER MENU",14}, {"GETTABLE",3}, {"HORIZONTAL MENU",2,true,true},
        {"PARLAY",6}, {"CALL",1}, {"DAMAGE",5}, {"AND",3,false,true},
        {"OR",3,false,true}, {"SPRITE OFF",0}, {"FIND ITEM",1}, {"PRINT RETURN",0},
        {"ECL CLOCK",1}, {"SAVE TABLE",3}, {"ADD NPC",2}, {"LOAD PIECES",3},
        {"PROGRAM",1}, {"WHO",1}, {"DELAY",0}, {"SPELL",3}, {"PROTECTION",1}, {"CLEAR BOX",0}
    }};
    if (op >= specs.size()) throw EclError("Unknown opcode " + std::to_string(op));
    return specs[op];
}

std::string unpack_ecl_text(std::span<const std::uint8_t> bytes)
{
    std::string text;
    unsigned buffer = 0, bits = 0;
    for (auto byte : bytes) {
        buffer = (buffer << 8) | byte; bits += 8;
        while (bits >= 6) {
            bits -= 6;
            const auto code = (buffer >> bits) & 63;
            if (code) text.push_back(static_cast<char>(code < 32 ? code + 64 : code));
        }
        buffer &= (1U << bits) - 1;
    }
    return text;
}

EclInstruction EclProgram::instruction(std::uint32_t address) const
{
    if (address < origin || address >= origin + raw_.size() - 2)
        throw EclError("Instruction address outside program");
    std::size_t cursor = address - origin + 2;
    const auto byte = [&]() {
        if (cursor >= raw_.size()) throw EclError("Truncated instruction operand");
        return raw_[cursor++];
    };
    EclInstruction result;
    result.address = static_cast<std::uint16_t>(address);
    result.opcode = byte();
    const auto& spec = ecl_opcode(result.opcode);
    const auto operand = [&]() {
        EclOperand arg;
        arg.tag = byte(); arg.value = byte();
        if (arg.tag == 1 || arg.tag == 2 || arg.tag == 3 || arg.tag == 129) {
            arg.value |= static_cast<std::uint16_t>(byte()) << 8;
        } else if (arg.tag == 128) {
            if (arg.value > raw_.size() - cursor) throw EclError("Truncated packed text");
            arg.text = unpack_ecl_text(std::span(raw_).subspan(cursor, arg.value));
            cursor += arg.value;
        } else if (arg.tag != 0) throw EclError("Unsupported operand tag " + std::to_string(arg.tag));
        return arg;
    };
    for (unsigned i = 0; i < spec.operands; ++i) result.operands.push_back(operand());
    if (spec.variable_list) {
        const auto& count = result.operands.back();
        if ((count.tag != 0 && count.tag != 2) || count.value > 255)
            throw EclError("Dynamic or invalid operand list length");
        const unsigned n = count.value;
        for (unsigned i = 0; i < n; ++i) result.operands.push_back(operand());
    }
    result.next = origin + static_cast<std::uint32_t>(cursor) - 2;
    return result;
}

EclProgram EclProgram::decode(std::span<const std::uint8_t> bytes, std::string source)
{
    try {
        if (bytes.size() < 2 || bytes.size() - 2 > 0x10000 - origin)
            throw EclError("Invalid ECL record size");
        EclProgram program;
        program.raw_.assign(bytes.begin(), bytes.end());
        program.source_ = source;
        auto cursor = origin;
        for (auto& target : program.entries_) {
            const auto entry = program.instruction(cursor);
            if (entry.opcode != 1 || entry.operands[0].tag >= 128)
                throw EclError("Invalid entry jump");
            target = entry.operands[0].value;
            cursor = entry.next;
        }
        program.body_start_ = cursor;
        for (auto target : program.entries_)
            if (target < cursor || target >= origin + bytes.size() - 2)
                throw EclError("Entry target outside program body");
        return program;
    } catch (const EclError& e) { throw EclError(source + ": " + e.what()); }
}
}
