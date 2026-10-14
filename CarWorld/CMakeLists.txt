# Copyright AStarship <https://astarship.net>.
cmake_minimum_required(VERSION 3.16)
project(iGeekCarWorldHeadless LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(CARWORLD_SANITIZE "Address/UB/float-cast sanitizers" OFF)
add_executable(carworld carworld.cpp)
add_executable(carworld_tests headless_tests.cpp)
add_executable(carworld_ppo_smoke ppo_smoke.cpp)
foreach(target carworld carworld_tests carworld_ppo_smoke)
  target_include_directories(${target} PRIVATE
    # Slot.h includes <_Config.h>; avoid the preserved UTF-16 legacy config.
    "${CMAKE_CURRENT_SOURCE_DIR}/../BikeWorld"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../ASCIICrabs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../ASCIICrabs/_Seams")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE
      -Werror=vla -Wno-deprecated-enum-enum-conversion
      -Werror=deprecated-enum-float-conversion $<$<CONFIG:Release>:-O2>)
    if(CARWORLD_SANITIZE)
      target_compile_options(${target} PRIVATE -O1 -g
        -fsanitize=address,undefined,float-cast-overflow
        -fno-sanitize-recover=all -fno-omit-frame-pointer)
      target_link_options(${target} PRIVATE
        -fsanitize=address,undefined,float-cast-overflow)
    endif()
  endif()
endforeach()
enable_testing()
add_test(NAME carworld_physics_routes COMMAND carworld_tests)
add_test(NAME carworld_ppo_interfaces COMMAND carworld_ppo_smoke)
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
  add_test(NAME carworld_cli COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cli_tests.py" "$<TARGET_FILE:carworld>")
  add_test(NAME carworld_evidence_validation COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verification_tests.py")
endif()
