#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <iostream>
#include <cassert>

#define TEST_CRASH(X)                   \
do {                                    \
    bool caught = false;                \
    try { X; }                          \
    catch (const SatSolverException& e) \
    {                                   \
        std::cout << e.what()           \
            << std::endl;               \
        caught = true;                  \
    }                                   \
    assert(caught);                     \
} while (0)

#define STR(s) std::string(#s)

#endif // TEST_UTIL_H
