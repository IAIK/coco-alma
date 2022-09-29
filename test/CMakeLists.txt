cmake_minimum_required(VERSION 3.10)

include(CTest)
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
  test_make_and_simplify
  test_add_clause_simplify
  test_add_clause_illegal)

foreach(TEST_NAME ${UNIT_SOLVER_TESTS})
add_test(NAME unit-solver:${TEST_NAME}
  COMMAND unit-solver ${TEST_NAME}
  WORKING_DIRECTORY .)
endforeach()

set(UNIT_PVS_SRC ../PropVarSet.cpp ../SatSolver.cpp unit-pvs.cpp)
add_executable(unit-pvs ${UNIT_PVS_SRC})
target_include_directories(unit-pvs PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(unit-pvs ${SOLVER_LIB_NAME})

set(UNIT_PVS_TESTS
  test_constructor
  test_ptr_xor
  test_ptr_plus_bias
  test_ptr_and)

foreach(TEST_NAME ${UNIT_PVS_TESTS})
    add_test(NAME unit-pvs:${TEST_NAME}
      COMMAND unit-pvs ${TEST_NAME}
      WORKING_DIRECTORY .)
endforeach()

set(UNIT_CELL_SRC ../Cell.cpp unit-cell.cpp)
add_executable(unit-cell ${UNIT_CELL_SRC})
target_include_directories(unit-cell PUBLIC "${PROJECT_SOURCE_DIR}")
# target_link_libraries(unit-cell ${SOLVER_LIB_NAME})

set(UNIT_CELL_TESTS
  test_and
  test_or
  test_xor
  test_nand
  test_nor
  test_xnor
  test_andnot
  test_ornot

  test_not
  test_buf

  )

foreach(TEST_NAME ${UNIT_CELL_TESTS})
    add_test(NAME unit-cell:${TEST_NAME}
      COMMAND unit-cell ${TEST_NAME}
      WORKING_DIRECTORY .)
endforeach()