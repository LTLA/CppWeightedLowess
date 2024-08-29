include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/afd902e992b720d1b3e106bc5e425a5768872265.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Avoid installing GoogleTest when installing this project.
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

option(CODE_COVERAGE "Enable coverage testing" OFF)
set(DO_CODE_COVERAGE OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(DO_CODE_COVERAGE ON)
endif()

macro(decorate_exe name) 
    target_link_libraries(
        ${name}
        gtest_main
        WeightedLowess
    )

    target_compile_options(${name} PRIVATE -Wall -Werror -Wpedantic -Wextra)

    if(DO_CODE_COVERAGE)
        target_compile_options(${name} PRIVATE -O0 -g --coverage)
        target_link_options(${name} PRIVATE --coverage)
    endif()

    gtest_discover_tests(${name})
endmacro()

add_executable(
    libtest 
    src/basic.cpp
    src/ties.cpp
    src/options.cpp
    src/SortBy.cpp
    src/divzero.cpp
    src/window.cpp
    src/robust.cpp
)
decorate_exe(libtest)

find_package(OpenMP)
if (OpenMP_FOUND) 
    # To test the SIMD macros.
    add_executable(
        omptest
        src/basic.cpp
        src/robust.cpp
    )
    decorate_exe(omptest)
    target_link_libraries(omptest OpenMP::OpenMP_CXX)
endif()
