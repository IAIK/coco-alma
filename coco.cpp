#include <iostream>
#include <cassert>
#include <iostream>

#include "Circuit.h"
#include "Simulator.h"

int main()
{
    Circuit circ("/home/vhadzic/Work/coco-alma/tmp/circuit.json", "top_module_d11");
    Simulator<verif_mode_t::MODE_GLITCH> simulator(circ);
    for (int i = 0; i < 100; i++)
    {
        simulator.step();
    }

    return 0;
}
