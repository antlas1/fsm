cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(test_fsm)

set (CMAKE_CXX_STANDARD 20)

add_executable(start_no_annot start_no_annot.cpp)
add_executable(start_with_annot start_with_annot.cpp)
add_executable(improved_with_annot improved_with_annot.cpp)