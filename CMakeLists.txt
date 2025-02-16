cmake_minimum_required(VERSION 3.17)

set(this simple_json_parser)
project(${this} VERSION 0.0.1 
	        DESCRIPTION "Simple JSON parsing and writing C++ library" 
		LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_POSITION_INDEPEDENT_CODE ON)

set(header_files include/simple_json.h)
set(source_files src/simple_json.cpp)

add_library(${this} STATIC ${header_files} ${source_files})

target_include_directories(${this} PUBLIC include)

include(CTest)

# enable_testing()

add_subdirectory(googletest)

add_subdirectory(tests)
