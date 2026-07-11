cmake_minimum_required(VERSION 3.15)

project(PPLEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenMP REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(ppl_engine ${SOURCES})


target_include_directories(ppl_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(ppl_engine PRIVATE OpenMP::OpenMP_CXX)