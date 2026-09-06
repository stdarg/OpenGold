#include "opengold/ecl_machine.h"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace opengold::por;
namespace {
using Bytes = std::vector<std::uint8_t>;
void check(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
template<class F> void rejects(F fn) { bool threw = false; try { fn(); } catch (const EclError&) { threw = true; } check(threw,"Expected ECL error"); }
Bytes record(const Bytes& body)
{
    Bytes b{0,0};
    for (int i = 0; i < 5; ++i) b.insert(b.end(), {1,1,0x14,0x99});
    b.insert(b.end(), body.begin(), body.end()); return b;
}
std::shared_ptr<const EclProgram> program(const Bytes& body)
{
    return std::make_shared<const EclProgram>(EclProgram::decode(record(body), "TEST:7"));
}
EclMachine machine(const Bytes& body)
{
    EclMachine vm(program(body)); vm.bind_variable(0x9700,0); check(vm.start(0),"Start"); return vm;
}
void decoding()
{
    check(unpack_ecl_text(Bytes{4,32,192}) == "ABC","Six-bit packed text");
    rejects([] { (void)EclProgram::decode({},"empty"); });
    rejects([] { (void)EclProgram::decode(Bytes(70000),"huge"); });
    auto bad = record({0}); bad[2] = 0;
    rejects([&] { (void)EclProgram::decode(bad,"entry"); });
    bad = record({0}); bad[4] = 0;
    rejects([&] { (void)EclProgram::decode(bad,"target"); });
    for (const Bytes body : {Bytes{17,128,3,4}, Bytes{17,1,0}, Bytes{17,9,0}, Bytes{255}, Bytes{43,1,0,0x97,1,0,0x97}}) {
        const auto p = program(body); rejects([&] { (void)p->instruction(0x9914); });
    }
    const auto p = program({52,0,1,0});
    check(p->instruction(0x9914).next == 0x9917,"PoR one-operand ECL CLOCK grammar");
    rejects([&] { (void)p->instruction(0x9918); });
    const auto literal = program({17,128,3,4,32,192,0});
    check(literal->instruction(0x9914).operands[0].text == "ABC","Inline text operand");
}
void arithmetic()
{
    auto vm = machine({9,2,255,255,1,0,0x97,4,0,1,1,0,0x97,1,0,0x97,0});
    check(vm.run(0).instructions == 0,"Zero budget");
    check(vm.run(1).state == EclState::running && vm.variable(0x9700) == 65535,"Budget yield");
    check(vm.run().state == EclState::completed && vm.variable(0x9700) == 0,"16-bit addition wrap");
    check(vm.start(4),"Restart another entry after completion");
    check(vm.run().state == EclState::completed,"Restart completes");
    for (const auto [op, expected] : std::array<std::array<unsigned,2>,4>{{{5,7},{7,30},{47,2},{48,11}}}) {
        auto n = machine({static_cast<std::uint8_t>(op),0,3,0,10,1,0,0x97,0});
        check(n.run().state == EclState::completed && n.variable(0x9700) == expected,"Arithmetic/bitwise operand semantics");
    }
    auto unbound = machine({9,0,1,1,1,0x97,0});
    const auto fail = unbound.run();
    check(fail.state == EclState::faulted && fail.diagnostic.find("TEST:7 @ 0x9914") != std::string::npos,"Unbound write with context");
    check(unbound.variable(0x9700) == 0,"Fault does not alter other variables");
    check(!unbound.start(0),"Fault cannot silently restart");
}
void control()
{
    // CALL subroutine at 9919, return to EXIT at 9918.
    auto call = machine({2,1,0x19,0x99,0,9,0,42,1,0,0x97,19});
    check(call.run().state == EclState::completed && call.variable(0x9700) == 42,"GOSUB/RETURN");
    auto underflow = machine({19}); check(underflow.run().state == EclState::faulted,"Stack underflow");
    auto recursion = machine({2,1,0x14,0x99}); check(recursion.run().state == EclState::faulted,"Stack limit");
    auto loop = machine({1,1,0x14,0x99});
    check(loop.run(5).state == EclState::running && loop.trace().size() == 5,"Loop yields");
    check(loop.run(1000000).state == EclState::faulted && loop.trace().size() == 64,"Total invocation limit and bounded trace");
    auto overlap = machine({1,1,0x15,0x99,0}); check(overlap.run().state == EclState::faulted,"Jump into operand");
    auto no_compare = machine({22,0}); check(no_compare.run().state == EclState::faulted,"Uninitialized comparison rejected");
    // False IF = skips the entire variable-length menu, without needing its destination binding.
    auto skip = machine({3,0,1,0,2,22,43,1,1,0x97,0,1,128,3,4,32,192,9,0,9,1,0,0x97,0});
    check(skip.run().state == EclState::completed && skip.variable(0x9700) == 9,"Variable-length conditional skip");
    for (unsigned op = 22; op <= 27; ++op) {
        const bool pass = op == 23 || op == 24 || op == 26;
        auto cond = machine({3,0,1,0,2,static_cast<std::uint8_t>(op),9,0,7,1,0,0x97,0});
        check(cond.run().state == EclState::completed && cond.variable(0x9700) == (pass ? 7 : 0),"Conditional relation");
    }
    // Indexed goto picks first target (991e); index 0 falls through to EXIT at 991d.
    for (std::uint8_t index : {0,1,2}) {
        auto indexed = machine({37,0,index,0,1,1,0x1e,0x99,0,0,9,0,8,1,0,0x97,0});
        check(indexed.run().state == EclState::completed && indexed.variable(0x9700) == (index == 1 ? 8 : 0),"Indexed goto and fallthrough");
    }
}
void suspension()
{
    auto vm = machine({43,1,0,0x97,0,2,128,3,4,32,192,128,3,4,32,192,17,1,0,0x97,0});
    auto result = vm.run();
    check(result.state == EclState::waiting && result.request->choices.size() == 2,"Menu suspends");
    const auto id = result.request->id;
    check(vm.run().instructions == 0 && vm.run().request->id == id,"Waiting does not reexecute menu");
    check(!vm.resume(id+1,0) && !vm.resume(id) && !vm.resume(id,2),"Stale/invalid menu replies");
    rejects([&] { vm.bind_variable(0x9700,99); });
    check(vm.resume(id,1) && vm.variable(0x9700) == 2,"Choice converted to one-based value");
    check(!vm.resume(id,0),"Duplicate response rejected");
    result = vm.run();
    check(result.state == EclState::waiting && result.request->text == "2","Output sees menu result");
    check(!vm.resume(result.request->id,0) && vm.resume(result.request->id),"Text reply validation");
    check(vm.run().state == EclState::completed,"Continuation completes");
    check(vm.start(0) && vm.run().request->id > id,"Request IDs never reused on restart");
    auto clear = machine({18,128,3,4,32,192,0});
    check(clear.run().request->clear,"PRINTCLEAR host intent");
    auto vertical = machine({21,1,0,0x97,128,3,4,32,192,0,1,128,3,4,32,192,0});
    const auto menu = vertical.run();
    check(menu.request && menu.request->vertical && menu.request->text == "ABC", "Vertical menu metadata");
    auto other = machine({9,0,13,1,0,0x97,0});
    check(other.run().state == EclState::completed && other.variable(0x9700) == 13 && vertical.variable(0x9700) == 0,
          "Independent VM variable state");
    auto unsupported = machine({36});
    check(unsupported.run().diagnostic.find("COMBAT") != std::string::npos,"Game command explicitly unsupported");
    auto native = machine({45,2,0,0});
    check(native.run().diagnostic.find("CALL") != std::string::npos,"DOS calls never executed");
}

struct Fixture {
    std::filesystem::path path = std::filesystem::temp_directory_path() / ("opengold-ecl-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    Fixture() { check(std::filesystem::create_directory(path),"Temporary directory"); }
    ~Fixture() { std::error_code ec; std::filesystem::remove_all(path,ec); }
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;
    void write(const std::string& filename, bool invalid = false) const {
        auto bytes = record({0});
        // Single literal-compressed DAX record, ID 7.
        Bytes dax{9,0,7,0,0,0,0,static_cast<std::uint8_t>(bytes.size()),0,static_cast<std::uint8_t>(bytes.size()+1),0};
        dax.push_back(static_cast<std::uint8_t>(bytes.size()-1)); dax.insert(dax.end(),bytes.begin(),bytes.end());
        if (invalid) dax.pop_back();
        std::ofstream out(path / filename,std::ios::binary);
        out.write(reinterpret_cast<const char*>(dax.data()),static_cast<std::streamsize>(dax.size()));
        check(bool(out),"Write fixture");
    }
};
void catalogs()
{
    Fixture f;
    rejects([&] { (void)EclCatalog::load(f.path); });
    f.write("ecl1.dax"); f.write("ECL2.DAX");
    std::shared_ptr<const EclProgram> held;
    { const auto c = EclCatalog::load(f.path); check(c.all().size() == 2,"All banks loaded"); held = c.find({"ECL1.DAX",7}); check(!c.find({"ECL1.DAX",8}),"Missing script explicit"); }
    EclMachine vm(held); check(vm.start(4) && vm.run().state == EclState::completed,"Program outlives catalog");
    f.write("ECL2.DAX",true); rejects([&] { (void)EclCatalog::load(f.path); });
    check(held->source() == "ECL1.DAX:7","Failed reload leaves existing program valid");
    if (const auto directory = std::getenv("OPENGOLD_GAME_DIR")) {
        const auto c = EclCatalog::load(directory);
        std::cout << "Installed ECL entry tables validated: " << c.all().size() << '\n';
    }
}
}
int main()
{
    try { decoding(); arithmetic(); control(); suspension(); catalogs(); std::cout << "ECL tests passed.\n"; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
