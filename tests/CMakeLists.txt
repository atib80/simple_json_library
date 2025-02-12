cmake_minimum_required(VERSION 3.17)

set(this_tests simple_json_parser_tests)
project(${this_tests})

set(header_files ../include/simple_json.h)
set(source_files tests.cpp)

add_executable(${this_tests} ${header_files} ${source_files})
target_include_directories(${this_tests} PUBLIC ../include)
target_link_libraries(${this_tests} PRIVATE simple_json_parser)
target_link_libraries(${this_tests} PUBLIC gtest_main)

add_test(NAME tests COMMAND ${this_tests})