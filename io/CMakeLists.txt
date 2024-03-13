cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(io)
add_subdirectory(src/lib)
add_subdirectory(src/main)
add_subdirectory(src/test)
