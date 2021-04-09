# see https://cmake.org/cmake/help/v3.19/guide/tutorial/index.html

cmake_minimum_required(VERSION 3.19)
project(SchokoVM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# https://cmake.org/cmake/help/latest/command/add_compile_options.html
if (MSVC)
    add_compile_options(/W4 /WX) # warning level 4 and all warnings as errors
else ()
    add_compile_options(-Wall -Wextra -pedantic -Werror
            # https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags
            -Wdouble-promotion -Wformat=2 -Wformat-truncation -Wundef -fno-common -Wconversion

            # from looking at man g++
            -fanalyzer
            -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined
            )
endif ()

link_libraries(asan ubsan)

# I'm not entirely sure how to use add_subdirectory(src) and we only have one executable anyway
add_executable(SchokoVM
        src/main.cpp
        src/types.hpp
        src/classfile.hpp
        src/parser.cpp src/parser.hpp
        )

enable_testing()
# https://cmake.org/cmake/help/latest/command/add_test.html
# https://cmake.org/cmake/help/latest/guide/tutorial/index.html#id11
function(do_test name)
    add_test(NAME ${name} COMMAND SchokoVM --test "../tests/${name}.java")
endfunction(do_test)

do_test(Empty)
