cmake_minimum_required(VERSION 3.30)
project(SimpleJSONParser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

add_executable(SimpleJSONParser tests/main.cpp
        include/simple_json.h)

target_include_directories(SimpleJSONParser PUBLIC include)
