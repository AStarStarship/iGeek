cmake_minimum_required(VERSION 2.6)
project("HBM")
# Use C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# The source files are in the HBM directory
add_executable(HBM
  main.cpp
  HRM.cpp
  hrm.h
)
target_include_directories(HBM PRIVATE
  /home/astar/AStarStarship/iGeek
  /home/astar/AStarStarship/ASCIICrabs
)