#include <iostream>
#include <cassert>
#include <iostream>

#include "Circuit.h"
#include "Simulator.h"

template <verif_mode_t mode>
void show_value(Simulator<mode>& sim, const std::string& name, uint64_t up, uint64_t down)
{
    auto x = sim[name][{up, down}];
    uint64_t val = 0LU;
    for (uint64_t i = 0; i < x.size(); i++)
    { val |= (uint64_t)((*x[i]).stable_val() & 1) << i; }
    std::cout << name << " " << x.size() << " " << val << std::endl;
}

template <verif_mode_t mode>
void show_value(Simulator<mode>& sim, const std::string& name)
{
    auto x = sim[name];
    show_value(sim, name, x.size() - 1, 0);
}

int main()
{
    constexpr verif_mode_t mode = verif_mode_t::MODE_GLITCH;
    using Simulator = Simulator<mode>;
    using Value = Value<mode>;

    Circuit circ("/home/vhadzic/Work/coco-alma/tmp/circuit.json", "top_module_d11");
    Simulator sim(circ);

    sim.prepare_cycle();
    sim["i_reset"][0] = 1;
    sim.step_cycle();
    sim.step();
    sim.prepare_cycle();
    sim["i_reset"][0] = 0;

    sim["i_enc_dec"][0] = 0;
    sim["i_r"][{47, 0}] = 0;

    sim["i_pt1"] = 0xf2ab9aeb45079458LU;
    sim["i_pt2"] = 0xf2ab9aeb45079458LU;

    sim["i_key1"][{ 31,  0}] = 0xb101598e;
    sim["i_key1"][{ 63, 32}] = 0x29cfa5cc;
    sim["i_key1"][{ 95, 64}] = 0x2fa459bd;
    sim["i_key1"][{127, 96}] = 0xfe7e5441;

    sim["i_key2"][{ 31,  0}] = 0xb101598e;
    sim["i_key2"][{ 63, 32}] = 0x29cfa5cc;
    sim["i_key2"][{ 95, 64}] = 0x2fa459bd;
    sim["i_key2"][{127, 96}] = 0xfe7e5441;

    show_value(sim, "i_key1", 127, 64);
    show_value(sim, "i_key2", 127, 64);
    show_value(sim, "i_key1", 63, 0);
    show_value(sim, "i_key2", 63, 0);


    sim["i_load"] = 1;
    sim["i_start"] = 0;
    sim.step_cycle();

    sim.prepare_cycle();
    sim["i_load"] = 0;
    sim["i_start"] = 1;
    sim.step_cycle();

    uint32_t cycles = 0;


    while ((*sim["o_done"][0]).stable_val() != true)
    {
        show_value(sim, "i_reset");
        show_value(sim, "i_load");
        show_value(sim, "i_start");
        show_value(sim, "next_state");
        show_value(sim, "state");
        show_value(sim, "sel4");
        show_value(sim, "key0_next1", 63, 0);
        show_value(sim, "key0_next2", 63, 0);
        show_value(sim, "key1_next1", 63, 0);
        show_value(sim, "key1_next2", 63, 0);

        show_value(sim, "i_key1", 127, 64);
        show_value(sim, "i_key2", 127, 64);
        show_value(sim, "i_key1", 63, 0);
        show_value(sim, "i_key2", 63, 0);

        sim.step();
        cycles += 1;
        std::cout << cycles << std::endl;
    }

    return 0;
}
