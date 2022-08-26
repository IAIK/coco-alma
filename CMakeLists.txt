cmake_minimum_required(VERSION 3.10)
project(coco-cpp)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(COCO_SOURCES coco.cpp SatSolver.cpp PropVarSet.cpp)
set(COCO_HEADERS coco.cpp SatSolver.h PropVarSet.h)

add_executable(coco-cpp ${COCO_SOURCES})

include(ExternalProject)
set(BACKEND CADICAL)
# set(BACKEND CRYPTOMINISAT)

if("${BACKEND}" STREQUAL "CRYPTOMINISAT")
    message("Adding external Cryptominisat target")
    set(SOLVER_NAME cryptominisat)
    set(SOLVER_DIR ${PROJECT_SOURCE_DIR}/${SOLVER_NAME})
    set(SOLVER_SRC_DIR ${SOLVER_DIR}/src)
    set(SOLVER_BUILD_DIR ${SOLVER_DIR}/build)
    set(SOLVER_OPTIONS -DIPASIR=ON -DSTATICCOMPILE=OFF -DNOZLIB=ON)
    set(SOLVER_CONFIG_CMD cmake -B ${SOLVER_BUILD_DIR} ${SOLVER_OPTIONS})
    set(SOLVER_LIB_NAME lib${SOLVER_NAME})
    set(SOLVER_LIB_PATH ${SOLVER_BUILD_DIR}/lib/libipasircryptominisat5.so)
    set(SOLVER_LIB_LINKAGE SHARED)
elseif("${BACKEND}" STREQUAL "CADICAL")
    message("Adding external Cadical target")
    set(SOLVER_NAME cadical)
    set(SOLVER_DIR ${PROJECT_SOURCE_DIR}/${SOLVER_NAME})
    set(SOLVER_SRC_DIR ${SOLVER_DIR}/src)
    set(SOLVER_BUILD_DIR ${SOLVER_DIR}/build)
    set(SOLVER_OPTIONS )
    set(SOLVER_CONFIG_CMD ./configure)
    set(SOLVER_LIB_NAME lib${SOLVER_NAME})
    set(SOLVER_LIB_PATH ${SOLVER_BUILD_DIR}/libcadical.a)
    set(SOLVER_LIB_LINKAGE STATIC)
else()
    message(FATAL_ERROR "No valid backend provided, aborting.")
    die()
endif()

ExternalProject_Add(${SOLVER_NAME}
  PREFIX ${SOLVER_DIR}
  SOURCE_DIR "${SOLVER_SRC_DIR}"
  CONFIGURE_COMMAND cd ${SOLVER_DIR} && ${SOLVER_CONFIG_CMD}
  BUILD_COMMAND     cd ${SOLVER_BUILD_DIR} && make -j4
  BUILD_BYPRODUCTS ${SOLVER_LIB_PATH}
  INSTALL_COMMAND   ""
  )
add_library(${SOLVER_LIB_NAME} ${SOLVER_LIB_LINKAGE} IMPORTED)
set_target_properties(${SOLVER_LIB_NAME} PROPERTIES
  IMPORTED_LOCATION "${SOLVER_LIB_PATH}"
  LINKER_LANGUAGE CXX)

add_dependencies(coco-cpp ${SOLVER_NAME})
target_link_libraries(coco-cpp ${SOLVER_LIB_NAME})

add_subdirectory(test)