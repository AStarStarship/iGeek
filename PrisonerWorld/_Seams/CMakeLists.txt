cmake_minimum_required(VERSION 2.6)
project("PrisonerWorld")
# Use C++11 standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# The source files are two directories up from _Seams
add_executable(PrisonerWorld
  ../_Main.cpp
  ../PrisonerWorld.cpp
  ../PrisonerWorld.h
)
target_include_directories(PrisonerWorld PRIVATE
  /home/astar/AStarStarship/iGeek
  /home/astar/AStarStarship/ASCIICrabs
)