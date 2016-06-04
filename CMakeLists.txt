cmake_minimum_required(VERSION 3.5)
project(Test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp include/linda.h tests/test.c include/linda.c)
add_executable(UXP1A ${SOURCE_FILES})