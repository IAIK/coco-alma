cmake_minimum_required(VERSION 3.10)

include(CTest)

###### Unit test suite for the SatSolver class ######

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

###### Unit test suite for the PropVarSet class ######

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

###### Unit test suite for the Cell class eval function ######

set(UNIT_CELL_SRC ../Cell.cpp unit-cell.cpp)
add_executable(unit-cell ${UNIT_CELL_SRC})
target_include_directories(unit-cell PUBLIC "${PROJECT_SOURCE_DIR}")
# target_link_libraries(unit-cell ${SOLVER_LIB_NAME})

set(UNIT_CELL_TESTS
  test_and_gate
  test_or_gate
  test_xor_gate
  test_nand_gate
  test_nor_gate
  test_xnor_gate
  test_andnot_gate
  test_ornot_gate

  test_not_gate
  test_buf_gate

  test_mux_gate
  test_nmux_gate

  test_dff_p_reg
  test_dff_n_reg

  test_dff_pp0_reg
  test_dff_pp1_reg
  test_dff_pn0_reg
  test_dff_pn1_reg
  test_dff_np0_reg
  test_dff_np1_reg
  test_dff_nn0_reg
  test_dff_nn1_reg

  test_dffe_pp_reg
  test_dffe_pn_reg
  test_dffe_np_reg
  test_dffe_nn_reg

  test_dffe_pp0p_reg
  test_dffe_pp1p_reg
  test_dffe_pn0p_reg
  test_dffe_pn1p_reg
  test_dffe_pp0n_reg
  test_dffe_pp1n_reg
  test_dffe_pn0n_reg
  test_dffe_pn1n_reg
  test_dffe_np0p_reg
  test_dffe_np1p_reg
  test_dffe_nn0p_reg
  test_dffe_nn1p_reg
  test_dffe_np0n_reg
  test_dffe_np1n_reg
  test_dffe_nn0n_reg
  test_dffe_nn1n_reg
  )

foreach(TEST_NAME ${UNIT_CELL_TESTS})
    add_test(NAME unit-cell:${TEST_NAME}
      COMMAND unit-cell ${TEST_NAME}
      WORKING_DIRECTORY .)
endforeach()

###### Unit test suite for the Simulator class using small verilog designs ######

set(UNIT_SIM_GADGETS_SRC ../Simulator.cpp ../Circuit.cpp ../Cell.cpp ../SatSolver.cpp ../Value.cpp ../PropVarSet.cpp unit-sim-gadgets.cpp)
add_executable(unit-sim-gadgets ${UNIT_SIM_GADGETS_SRC})
target_include_directories(unit-sim-gadgets PUBLIC "${PROJECT_SOURCE_DIR}")
target_include_directories(unit-sim-gadgets PUBLIC "${PROJECT_SOURCE_DIR}/json/single_include/nlohmann")
target_link_libraries(unit-sim-gadgets ${SOLVER_LIB_NAME})

set(UNIT_SIM_DOM_AND_TEST_2_R dom_and random "gadgets/synthd/dom_and_2.json")
set(UNIT_SIM_DOM_AND_TEST_3_R dom_and random "gadgets/synthd/dom_and_3.json")
set(UNIT_SIM_DOM_AND_TEST_4_R dom_and random "gadgets/synthd/dom_and_4.json")

set(UNIT_SIM_DOM_AND_TEST_2_P dom_and pvs "gadgets/synthd/dom_and_2.json")
set(UNIT_SIM_DOM_AND_TEST_3_P dom_and pvs "gadgets/synthd/dom_and_3.json")
set(UNIT_SIM_DOM_AND_TEST_4_P dom_and pvs "gadgets/synthd/dom_and_4.json")

set(UNIT_SIM_GADGET_TESTS
  UNIT_SIM_DOM_AND_TEST_2_R
  UNIT_SIM_DOM_AND_TEST_3_R
  UNIT_SIM_DOM_AND_TEST_4_R

  UNIT_SIM_DOM_AND_TEST_2_P
  UNIT_SIM_DOM_AND_TEST_3_P
  UNIT_SIM_DOM_AND_TEST_4_P
  )

foreach(TEST_VAR ${UNIT_SIM_GADGET_TESTS})
    set(TEST_ARGS ${${TEST_VAR}})
    list(JOIN TEST_ARGS ":" TEST_STR)
    add_test(NAME unit-sim-gadgets:${TEST_STR}
      COMMAND unit-sim-gadgets ${TEST_ARGS}
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test")
endforeach()