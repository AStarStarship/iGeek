cmake_minimum_required (VERSION 3.10)
project (iGeekCardsWorld)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# The ASCIICrabs core is a sibling directory.
set (CRABS_DIR "${CMAKE_SOURCE_DIR}/../../ASCIICrabs")

add_executable (iGeekCardsWorld _Seams/_Main.cpp)

target_include_directories (iGeekCardsWorld PRIVATE
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/_Seams"
  "${CRABS_DIR}"
)
