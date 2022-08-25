#include <iostream>
#include <cassert>
#include <iostream>

extern "C" {
#include "ipasir.h"
}

int main() {
    void* s = ipasir_init();
    ipasir_add(s, 1);
    ipasir_add(s, 0);

    int ret = ipasir_solve(s);
    assert(ret == 10);

    int val = ipasir_val(s, 1);
    std::cout << val << std::endl;

    ipasir_release(s);
    return 0;
}
