cmake_minimum_required(VERSION 2.6)
project("Pac-woman")

add_definitions("-std=c++11)

include_directories("${PROJECT_BINARY_DIR}/../../../../SFML/include/SFML")

set(SRC main.cc)

add_executable(pacwoman ${SRC})

target_link_libraries(pacwoman sfml-sfml)
target_link_libraries(pacwoman sfml-window)
target_link_libraries(pacwoman sfml-graphics)
target_link_libraries(pacwoman sfml-audio)
target_link_libraries(pacwoman sfml-network)
