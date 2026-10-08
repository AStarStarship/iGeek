cmake_minimum_required(VERSION 3.10)
project(Vireworld CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/..
  ${CMAKE_SOURCE_DIR}/../../ASCIICrabs
)
add_executable(Vireworld _Main.cpp)
