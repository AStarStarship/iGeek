cmake_minimum_required (VERSION 3.10)
project (iGeek)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# The ASCIICrabs core is a sibling directory.
set (CRABS_DIR "${CMAKE_SOURCE_DIR}/../../ASCIICrabs")

# Single executable, single translation unit: _Main.cpp pulls _Tests.hxx, which
# rolls up the RL/agent core + every world's seam units into one TTestTree.
#
# SEAM selects which world's code the preprocessor admits (see _Seams.h):
#   * no -DSEAM          -> SEAM_N (the last world's RELEASE): everything sown,
#                           the full tree runs (every world's Release).
#   * -DSEAM=<W>_CORE    -> that world's unit-test tree (Core); higher worlds'
#                           code is blocked off by their `#if SEAM >=` gates.
#   * -DSEAM=<W>_RELEASE -> that world's official output (its Release); higher
#                           worlds blocked off, lower worlds run as tests.
# This reaches the C++ preprocessor so `#if SEAM == IGEEK_<W>_RELEASE`
# dispatches the right seam in each world's _Tests.hxx.
set (SEAM "" CACHE STRING "Seam index to build at (default: SEAM_N = full sown tree)")
if (SEAM)
  add_compile_definitions (SEAM=${SEAM})
endif ()

add_executable (iGeek ./_Main.cpp)

target_include_directories (iGeek PRIVATE
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/../"
  "${PROJECT_SOURCE_DIR}/../Classical"
  "${CRABS_DIR}"
)
