cmake_minimum_required(VERSION 3.26)

project(TransportTycoon)

add_executable(terrain "src/terrain.cpp")

target_compile_definitions(terrain PUBLIC PROJECT_PATH="${CMAKE_SOURCE_DIR}")

