cmake_minimum_required(VERSION 3.14)

project(WeightedLowess
    VERSION 2.1.2
    DESCRIPTION "A C++ port of the limma::weightedLowess function"
    LANGUAGES CXX)

add_library(WeightedLowess INTERFACE)
add_library(ltla::WeightedLowess ALIAS WeightedLowess)

include(GNUInstallDirs)
target_include_directories(WeightedLowess INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ltla_WeightedLowess>)

target_compile_features(WeightedLowess INTERFACE cxx_std_17)

# Dependencies
option(WEIGHTEDLOWESS_FETCH_EXTERN "Automatically fetch WeightedLowess's external dependencies." ON)
if(WEIGHTEDLOWESS_FETCH_EXTERN)
    add_subdirectory(extern)
else()
    find_package(ltla_subpar 0.3.1 CONFIG REQUIRED)
endif()

target_link_libraries(WeightedLowess INTERFACE ltla::subpar)

# Testing.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(WEIGHTEDLOWESS_TESTS "Build WeightedLowess's test suite." ON)
else()
    option(WEIGHTEDLOWESS_TESTS "Build WeightedLowess's test suite." OFF)
endif()

if(WEIGHTEDLOWESS_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ltla_WeightedLowess)

install(TARGETS WeightedLowess
    EXPORT WeightedLowessTargets)

install(EXPORT WeightedLowessTargets
    FILE ltla_WeightedLowessTargets.cmake
    NAMESPACE ltla::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_WeightedLowess)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_WeightedLowessConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_WeightedLowess)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ltla_WeightedLowessConfigVersion.cmake
    COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ltla_WeightedLowessConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ltla_WeightedLowessConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ltla_WeightedLowess)
