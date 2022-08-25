cmake_minimum_required(VERSION 3.23)

set(UNIT_SOLVER_SRC ../SatSolver.cpp unit-solver.cpp)
add_executable(unit-solver ${UNIT_SOLVER_SRC})
target_include_directories(unit-solver PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(unit-solver ${SOLVER_LIB_NAME})

add_test(unit-solver unit-solver)

set(UNIT_PVS_SRC ../PropVarSet.cpp ../SatSolver.cpp unit-pvs.cpp)
add_executable(unit-pvs ${UNIT_PVS_SRC})
target_include_directories(unit-pvs PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(unit-pvs ${SOLVER_LIB_NAME})

add_test(unit-pvs unit-pvs)