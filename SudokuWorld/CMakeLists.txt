cmake_minimum_required(VERSION 2.6)
project(SudokuWorld)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(SudokuWorld
  main.cpp
)
target_include_directories(SudokuWorld PRIVATE
  /home/astar/AStarStarship/iGeek
  /home/astar/AStarStarship/ASCIICrabs
  /home/astar/AStarStarship/iGeek/HBM
)