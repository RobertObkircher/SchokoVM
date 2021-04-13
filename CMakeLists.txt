# see https://cmake.org/cmake/help/v3.19/guide/tutorial/index.html

cmake_minimum_required(VERSION 3.19)
project(SchokoVM)

find_package(Java 1.8 REQUIRED)
include(UseJava)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# https://cmake.org/cmake/help/latest/command/add_compile_options.html
if (MSVC)
    add_compile_options(/W4 /WX) # warning level 4 and all warnings as errors
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options(
            -Weverything
            -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-switch-enum -Wno-padded
            -fno-common
            -fsanitize=address
    )
    add_link_options(-fsanitize=address)
else ()
    add_compile_options(-Wall -Wextra -pedantic # -Werror
            # https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags
            -Wdouble-promotion -Wformat=2 -Wformat-truncation -Wundef -fno-common -Wconversion

            # from looking at man g++
            -fanalyzer
            -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined
            )
    add_link_options(
            -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined
    )
endif ()

# I'm not entirely sure how to use add_subdirectory(src) and we only have one executable anyway
add_executable(SchokoVM
        src/main.cpp
        src/types.hpp
        src/classfile.hpp
        src/parser.cpp src/parser.hpp
        src/args.cpp src/args.hpp
        src/jar.cpp src/jar.hpp
        )
target_link_libraries(SchokoVM zip)

# Note: using glob to create this list is not recommended
#file(GLOB JAVA_TEST_SOURCES CONFIGURE_DEPENDS tests/*.java)
set(JAVA_TEST_SOURCES
        tests/Empty.java
        tests/HelloWorld.java
        )

# https://cmake.org/cmake/help/latest/module/UseJava.html
add_jar(tests ${JAVA_TEST_SOURCES})
add_dependencies(SchokoVM tests)

enable_testing()
# https://cmake.org/cmake/help/latest/command/add_test.html
# https://cmake.org/cmake/help/latest/guide/tutorial/index.html#id11
function(do_test path)
    get_filename_component(name ${path} NAME_WE)
    add_test(NAME ${name} COMMAND SchokoVM --test ${Java_JAVA_EXECUTABLE} "out/${name}" --class-path tests.jar ${name})
endfunction(do_test)

foreach (source ${JAVA_TEST_SOURCES})
    do_test(${source})
endforeach ()
