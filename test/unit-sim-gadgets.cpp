#include "Simulator.h"
#include <random>
#include <bitset>
#include <filesystem>
namespace fs = std::filesystem;

template <verif_mode_t mode>
void random_init(Simulator<mode>& sim, const char* name, uint64_t size, std::mt19937_64& rand)
{
    uint32_t offset = 0;
    while (offset + 64 < size)
    {
        sim[name][{offset + 63, offset}] = rand();
        offset += 64;
    }
    sim[name][{size - 1, offset}] = ((1LU << (size - offset)) - 1) & rand();
}

template <verif_mode_t mode>
bool get_xor(Simulator<mode>& sim, const char* name)
{
    ValueViewVector<mode> view = sim[name];
    const uint64_t size = view.size();
    bool result = false;
    uint32_t offset = 0;
    while (offset + 64 < size)
    {
        std::bitset<64> bits(view[{offset + 63, offset}].as_uint64_t());
        result ^= bits.count() & 1;
        offset += 64;
    }
    {
        std::bitset<64> bits(view[{size - 1, offset}].as_uint64_t());
        result ^= bits.count() & 1;
    }
    return result;
}

int test_random_dom_and(const char* path)
{
    constexpr verif_mode_t mode = verif_mode_t::MODE_STABLE;
    using Simulator = Simulator<mode>;
    using Value = Value<mode>;
    using ValueView = ValueView<mode>;
    using ValueViewVector = ValueViewVector<mode>;
    constexpr uint32_t NUM_SAMPLES = 1000;

    Circuit circ(path, "dom_and");
    const uint32_t num_shares = circ["LeftDI"].size();
    const uint32_t num_masks = circ["RandomDI"].size();

    std::random_device rd;
    std::mt19937_64 rand(rd());
    for (uint32_t sample = 0; sample < NUM_SAMPLES; sample++)
    {
        Simulator sim(circ);
        sim.prepare_cycle();
        sim["RstRI"] = 1;

        random_init(sim, "LeftDI", num_shares, rand);
        random_init(sim, "RightDI", num_shares, rand);
        random_init(sim, "RandomDI", num_masks, rand);

        bool left_pre = get_xor(sim, "LeftDI");
        bool right_pre = get_xor(sim, "RightDI");
        bool expected_result = left_pre & right_pre;

        sim.step_cycle();
        sim.step();

        bool left_post = get_xor(sim, "LeftDI");
        bool right_post = get_xor(sim, "RightDI");
        bool received_result = get_xor(sim, "OutDO");
        assert(left_pre == left_post);
        assert(right_pre == right_post);
        assert(expected_result == received_result);
        std::cout << "Test result: " << left_pre << " & " << right_pre << " == " << received_result << std::endl;
    }
    return 0;
}

int test_pvs_dom_and(const char* path)
{
    constexpr verif_mode_t mode = verif_mode_t::MODE_GLITCH;
    using Simulator = Simulator<mode>;
    using Value = Value<mode>;
    using ValueView = ValueView<mode>;
    using ValueViewVector = ValueViewVector<mode>;

    Circuit circ(path, "dom_and");
    const uint32_t num_shares = circ["LeftDI"].size();
    const uint32_t num_masks = circ["RandomDI"].size();

    Simulator sim(circ);
    sim.allocate_secrets({1, 0}, num_shares);
    sim.allocate_masks({num_masks - 1, 0});
    sim.prepare_cycle();
    sim["RstRI"] = 1;
    sim["LeftDI"] = sim.ith_secret({num_shares - 1, 0}, 0);
    sim["RightDI"] = sim.ith_secret({num_shares - 1, 0}, 1);
    sim["RandomDI"] = sim.masks({num_masks - 1, 0});
    sim.step_cycle();
    sim.step();
    std::cout << "LeftDI: " << sim["LeftDI"] << std::endl;
    std::cout << "RightDI: " << sim["RightDI"] << std::endl;
    std::cout << "RandomDI: " << sim["RandomDI"] << std::endl;
    std::cout << "OutDO: " << sim["OutDO"] << std::endl;
    std::string out_name = path;
    out_name.erase(out_name.rfind(".json"));
    sim.dump_vcd(out_name + ".pre.vcd");
    SatSolver::state_t res = sim.m_solver.check();
    assert(res == SatSolver::STATE_SAT);
    sim.dump_vcd(out_name + ".sat.vcd");
    return 0;
}

int main(int argc, const char* argv[])
{
    if(argc != 4) return 1;
    std::string gadget_type(argv[1]);
    if (gadget_type == "dom_and")
    {
        std::string test_type(argv[2]);
        if (test_type == "random")
            return test_random_dom_and(argv[3]);
        else if (test_type == "pvs")
            return test_pvs_dom_and(argv[3]);
        return 3;
    }

    return 2;
}