#include <iostream>
#include <iomanip>
#include <cassert>
#include <iostream>

#include "Circuit.h"
#include "Simulator.h"

template <verif_mode_t mode>
void show_value(Simulator<mode>& sim, const std::string& name, uint64_t up, uint64_t down)
{
    auto x = sim[name][{up, down}];
    std::ios_base::fmtflags flags(std::cout.flags());
    std::cout << name << "[" << up << ":" << down << "] = ";
    std::cout << std::hex << std::setfill('0');
    const int leftover = x.size() % 64;
    const int num_blocks = (x.size() / 64) + (leftover != 0);
    const int first_block_size = leftover == 0 ? 64 : leftover;

    int block_size = first_block_size;
    int block_pos = num_blocks;
    do
    {
        const uint64_t offset = (block_pos - 1) * 64;
        const int print_size = block_size / 4 + (block_size % 4 != 0);
        uint64_t val = 0;
        for (uint64_t i = 0; i < block_size; i++)
        { val |= (uint64_t)((x[offset + i].get()).stable_val() & 1) << i; }
        std::cout << std::setw(print_size);
        std::cout << val;

        block_size = 64;
        block_pos -= 1;
    } while (block_pos != 0);
    std::cout.flags(flags);
    std::cout << std::endl;
}

template <verif_mode_t mode>
void show_value(Simulator<mode>& sim, const std::string& name)
{
    auto x = sim[name];
    show_value(sim, name, x.size() - 1, 0);
}

int main()
{
    constexpr verif_mode_t mode = verif_mode_t::MODE_STABLE;
    using Simulator = Simulator<mode>;
    using Value = Value<mode>;

    Circuit circ("/home/vhadzic/Work/coco-alma/tmp/circuit.json", "top_module_d11");
    Simulator sim(circ);

    sim.allocate_secrets({191, 0}, 2);

    sim.prepare_cycle();
    sim["i_reset"][0] = 1;
    sim.step_cycle();
    sim.step();
    sim.prepare_cycle();

    sim.allocate_masks({47, 0});
    sim["i_reset"][0] = 0;

    sim["i_enc_dec"][0] = 0;
    sim["i_r"][{47, 0}] = sim.masks({47, 0});

    // sim["i_pt1"] = 0xf2ab9aeb45079458LU;
    // sim["i_pt2"] = 0xf2ab9aeb45079458LU;
    sim["i_pt1"] = sim.ith_share({191, 128}, 0);
    sim["i_pt2"] = sim.ith_share({191, 128}, 1);

//    sim["i_key1"][{ 31,  0}] = 0xb101598e;
//    sim["i_key1"][{ 63, 32}] = 0x29cfa5cc;
//    sim["i_key1"][{ 95, 64}] = 0x2fa459bd;
//    sim["i_key1"][{127, 96}] = 0xfe7e5441;
//
//    sim["i_key2"][{ 31,  0}] = 0xb101598e;
//    sim["i_key2"][{ 63, 32}] = 0x29cfa5cc;
//    sim["i_key2"][{ 95, 64}] = 0x2fa459bd;
//    sim["i_key2"][{127, 96}] = 0xfe7e5441;

    sim["i_key1"] = sim.ith_share({127, 0}, 0);
    sim["i_key2"] = sim.ith_share({127, 0}, 1);

    sim["i_load"] = 1;
    sim["i_start"] = 0;
    sim.step_cycle();

    sim.prepare_cycle();
    sim["i_load"] = 0;
    sim["i_start"] = 1;
    sim.step_cycle();

    uint32_t cycles = 0;

    while ((sim["o_done"][0].get()).stable_val() != true)
    {
        show_value(sim, "i_reset");
        show_value(sim, "i_load");
        show_value(sim, "i_start");
        show_value(sim, "next_state");
        show_value(sim, "state");
        show_value(sim, "sel4");
        show_value(sim, "rnd_cnt");
        sim.step();
        cycles += 1;
        std::cout << cycles << std::endl;
        if (cycles == 6) break;
    }

    sim.step();

    return 0;
}
