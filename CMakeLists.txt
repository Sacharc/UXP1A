cmake_minimum_required(VERSION 3.0.2)
project(Test)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        main.c
        include/linda.h
        tests/test.c
        include/linda.c
        include/dynamic_va_list.h
        include/dynamic_va_list.h)
add_executable(UXP1A ${SOURCE_FILES})

##Tests
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
#
#set(TEST_FILES
#        tests/linda_float_functional_tests.cc
#        tests/linda_int_functional_tests.cc
#        include/linda.c
#        include/linda.h
#        tests/linda_string_functional_tests.cc)
#
#add_executable(runTests
#        ${TEST_FILES}
#        )
#
#target_link_libraries(runTests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES}  pthread)