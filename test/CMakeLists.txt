cmake_minimum_required(VERSION 3.10)

set(UNIT_SOLVER_SRC ../SatSolver.cpp unit-solver.cpp)
add_executable(unit-solver ${UNIT_SOLVER_SRC})
target_include_directories(unit-solver PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(unit-solver ${SOLVER_LIB_NAME})

set(UNIT_SOLVER_TESTS
  test_new_var
  test_new_vars
  test_make_xor
  test_make_and
  test_add_clause
  test_make_xor_simplify
  test_make_and_simplify)
foreach(TEST_NAME ${UNIT_SOLVER_TESTS})
add_test(NAME unit-solver:${TEST_NAME}
  COMMAND unit-solver ${TEST_NAME}
  WORKING_DIRECTORY .)
endforeach()

set(UNIT_PVS_SRC ../PropVarSet.cpp ../SatSolver.cpp unit-pvs.cpp)
add_executable(unit-pvs ${UNIT_PVS_SRC})
target_include_directories(unit-pvs PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(unit-pvs ${SOLVER_LIB_NAME})

add_test(unit-pvs unit-pvs)