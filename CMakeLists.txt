cmake_minimum_required(VERSION 3.14)

project(WeightedLowess
    VERSION 1.0.0
    DESCRIPTION "A C++ port of the limma::weightedLowess function"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(WeightedLowess INTERFACE)

target_include_directories(WeightedLowess INTERFACE include/)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
