# see https://cmake.org/cmake/help/v3.19/guide/tutorial/index.html

cmake_minimum_required(VERSION 3.19)
project(SchokoVM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Java 1.8 REQUIRED)
include(UseJava)

find_package(libzip)

# https://cmake.org/cmake/help/latest/command/add_compile_options.html
if (MSVC)
    add_compile_options(/W4 /WX) # warning level 4 and all warnings as errors
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    add_compile_options(
            -Wall -Wextra -pedantic
#            -Wold-style-cast
            -Wdouble-promotion -Wformat=2 -Wundef -fno-common -Wconversion
            -Wno-nullability-extension
            -fno-common
            # Catch signed integer overflows
            -ftrapv
            -fsanitize=address
    )
    add_link_options(-fsanitize=address)
else ()
    add_compile_options(-Wall -Wextra -pedantic # -Werror
            # https://interrupt.memfault.com/blog/best-and-worst-gcc-clang-compiler-flags
            -Wdouble-promotion -Wformat=2 -Wformat-truncation -Wundef -fno-common -Wconversion

            # from looking at man g++
            # -fanalyzer # unfortunately this is very slow
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
        src/interpreter.cpp src/interpreter.hpp
        src/opcodes.hpp
        src/future.hpp
        src/object.cpp src/object.hpp
        )

if (libzip_FOUND)
    target_link_libraries(SchokoVM libzip::zip)
else ()
    # hope that it's on the standard search paths
    target_link_libraries(SchokoVM zip)
endif ()

# Note: using glob to create this list is not recommended
#file(GLOB JAVA_TEST_SOURCES CONFIGURE_DEPENDS tests/*.java)
set(JAVA_TEST_SOURCES
        tests/Bitwise.java
        tests/Empty.java
#        tests/HelloWorld.java
        tests/Fields.java
        tests/InvokeStatic.java
#        tests/KarelTheRobot.java
        tests/ReferenceComparisons.java
        tests/Switch.java
        tests/UnitBoolean.java
        )

set(JAVA_TEST_GENERATED_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntShiftLeft.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntShiftRight.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntShiftRightU.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongShiftLeft.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongShiftRight.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongShiftRightU.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ComparisonsInt.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ComparisonsLong.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/Conversions.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/Goto.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/InvokeStaticPermutations.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/Wide.java
        )

set(TESTS_GENERATED ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated)

add_custom_command(
        OUTPUT ${JAVA_TEST_GENERATED_SOURCES} ${TESTS_GENERATED}/Generator.class
        COMMAND ${Java_JAVAC_EXECUTABLE} ${TESTS_GENERATED}/Generator.java
        COMMAND ${Java_JAVA_EXECUTABLE} -classpath ${TESTS_GENERATED}/ Generator ${TESTS_GENERATED}/out
        DEPENDS ${TESTS_GENERATED}/Generator.java
        VERBATIM
)

# https://cmake.org/cmake/help/latest/module/UseJava.html
add_jar(tests ${JAVA_TEST_SOURCES} ${JAVA_TEST_GENERATED_SOURCES})
add_dependencies(SchokoVM tests)

enable_testing()
# https://cmake.org/cmake/help/latest/command/add_test.html
# https://cmake.org/cmake/help/latest/guide/tutorial/index.html#id11
function(do_test path)
    get_filename_component(name ${path} NAME_WE)
    add_test(NAME ${name} COMMAND sh "${CMAKE_SOURCE_DIR}/compare.sh" ${Java_JAVA_EXECUTABLE} ${name})
endfunction(do_test)

foreach (source ${JAVA_TEST_SOURCES} ${JAVA_TEST_GENERATED_SOURCES})
    do_test(${source})
endforeach ()
