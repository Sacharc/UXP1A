cmake_minimum_required(VERSION 3.0.2)
project(Test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")

set(SOURCE_FILES main.c include/linda.h tests/test.c include/linda.c include/dynamic_va_list.h include/dynamic_va_list.h)
add_executable(UXP1A ${SOURCE_FILES})