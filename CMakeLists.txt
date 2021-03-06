cmake_minimum_required(VERSION 2.8)
project(Test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -D_GNU_SOURCE -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES
        include/linda.h
        include/linda.c
        include/dynamic_va_list.h
        include/dynamic_va_list.h)

add_library(linda SHARED ${SOURCE_FILES})
add_executable(UXP1A
        main.c
        ${SOURCE_FILES})

#Tests
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(TEST_FILES
        include/linda.c
        include/linda.h
        tests/linda_float_functional_tests.cc
        tests/linda_int_functional_tests.cc
        tests/linda_string_functional_tests.cc
        tests/info_string_match_string_equals_test.cc
        tests/tests.h)

add_executable(runTests
        ${TEST_FILES}
        )

target_link_libraries(runTests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)