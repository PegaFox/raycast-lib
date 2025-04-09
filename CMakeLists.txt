cmake_minimum_required(VERSION 3.30)

project(raycast-lib)

add_subdirectory(collision-lib)

add_library(raycast-lib STATIC raycast.cpp)

target_link_libraries(raycast-lib collider-lib)
