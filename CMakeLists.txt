# Copyright AStarship <https://astarship.net>.
# iGeek core framework build: the RL/agent layer self-test (seam binary).
cmake_minimum_required(VERSION 3.16)
project(iGeekCore LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(IGEEK_SANITIZE "Address/UB/float-cast sanitizers" OFF)

add_executable(igeeK_core _Seams/_Main.cpp)
target_include_directories(igeeK_core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/_Seams"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/../ASCIICrabs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../ASCIICrabs/_Seams")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(igeeK_core PRIVATE
    -Werror=vla -Wno-deprecated-enum-enum-conversion
    -Werror=deprecated-enum-float-conversion
    $<$<CONFIG:Release>:-O2>)
  if(IGEEK_SANITIZE)
    target_compile_options(igeeK_core PRIVATE -O1 -g
      -fsanitize=address,undefined,float-cast-overflow
      -fno-sanitize-recover=all -fno-omit-frame-pointer)
    target_link_options(igeeK_core PRIVATE
      -fsanitize=address,undefined,float-cast-overflow)
  endif()
endif()

enable_testing()
add_test(NAME igeeK_core COMMAND igeeK_core)
