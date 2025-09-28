include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/571930618fa96eabcd05b573285edbee9fc13bae.zip
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

macro(decorate_test target)
    target_link_libraries(
        ${target}
        gtest_main
        WeightedLowess
    )

    target_compile_options(${target} PRIVATE -Wall -Werror -Wpedantic -Wextra)

    if(DO_CODE_COVERAGE)
        target_compile_options(${target} PRIVATE -O0 -g --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endif()

    gtest_discover_tests(${target})
endmacro()

add_executable(
    libtest
    src/compute.cpp
    src/ties.cpp
    src/options.cpp
    src/SortBy.cpp
    src/divzero.cpp
    src/window.cpp
    src/robust.cpp
    src/interpolate.cpp
)
decorate_test(libtest)
