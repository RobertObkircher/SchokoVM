# see https://cmake.org/cmake/help/v3.19/guide/tutorial/index.html

# Generator expressions: https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
# $<$<CXX_COMPILER_ID:GNU>:-Wall> with AppleClang|Clang|GNU

cmake_minimum_required(VERSION 3.19)
project(SchokoVM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Java 11 EXACT REQUIRED COMPONENTS Runtime Development)
find_package(JNI 11 EXACT REQUIRED)
include(UseJava)

find_package(libzip)

if (APPLE)
    set(CMAKE_SHARED_MODULE_SUFFIX ".dylib")
endif ()

#
# Common compiler options
#

# For debugging
#set(CMAKE_VERBOSE_MAKEFILE on)
if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(
            -Wall -Wextra -pedantic
            #            -Wold-style-cast

            -Wdouble-promotion -Wconversion

            # We don't really use printf but whatever
            -Wformat=2

            # Warn if an undefined identifier is evaluated in an "#if" directive.
            -Wundef

            # no duplicate global variables
            -fno-common

            # not sure what this does
            $<$<CXX_COMPILER_ID:AppleClang>:-Wno-nullability-extension>

            # Catch signed integer overflows
            #-ftrapv

            # static analysis, unfortunately this is very slow
            # -fanalyzer
    )
endif ()

function(add_sanitizers target)
    if (MSVC)
    else ()
        target_compile_options(${target} PRIVATE
                -fsanitize=address
                $<$<NOT:$<CXX_COMPILER_ID:AppleClang>>:
                -fsanitize=pointer-compare
                -fsanitize=pointer-subtract
                -fsanitize=leak
                -fsanitize=undefined
                >
                )
        target_link_options(${target} PRIVATE
                -fsanitize=address
                $<$<NOT:$<CXX_COMPILER_ID:AppleClang>>:
                -fsanitize=pointer-compare
                -fsanitize=pointer-subtract
                -fsanitize=leak
                -fsanitize=undefined
                >
                )
    endif ()
endfunction(add_sanitizers)

#
# Executables and libraries
#

add_library(jvm SHARED
        src/types.hpp
        src/classfile.hpp
        src/parser.cpp src/parser.hpp
        src/args.cpp src/args.hpp
        src/zip.cpp src/zip.hpp
        src/interpreter.cpp src/interpreter.hpp
        src/opcodes.hpp
        src/future.hpp
        src/memory.cpp src/memory.hpp
        src/math.hpp
        src/classloading.cpp src/classloading.hpp
        src/util.cpp src/util.hpp
        src/native.cpp src/native.hpp
        src/jni.cpp
        src/jvm.cpp
        src/unsafe.cpp src/unsafe.hpp src/data.hpp)
add_sanitizers(jvm)
target_include_directories(jvm PRIVATE ${JNI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/jdk/include)
target_compile_options(jvm PRIVATE -Wno-unused-parameter)
target_compile_definitions(jvm PRIVATE LIB_EXTENSION="${CMAKE_SHARED_LIBRARY_SUFFIX}")
if (libzip_FOUND)
    target_link_libraries(jvm libzip::zip)
else ()
    # hope that it's on the standard search paths
    target_link_libraries(jvm zip)
endif ()


add_executable(SchokoVM src/main.cpp
        src/args.hpp
        src/interpreter.hpp
        )
add_sanitizers(SchokoVM)
target_include_directories(SchokoVM PRIVATE ${JNI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/jdk/include)
target_link_libraries(SchokoVM jvm)


add_library(NativeLib MODULE
        src/native-test.cpp
        )
target_link_libraries(jvm dl ffi)


#
# OpenJDK
#

file(REAL_PATH ${Java_JAVAC_EXECUTABLE} JDK_HOME)
get_filename_component(JDK_HOME ${JDK_HOME} DIRECTORY)
get_filename_component(JDK_HOME ${JDK_HOME} DIRECTORY)
message(STATUS "jdk: ${JDK_HOME}")

set(JDK_EXPLODED ${CMAKE_CURRENT_SOURCE_DIR}/jdk/exploded-modules)
if (NOT IS_DIRECTORY ${JDK_EXPLODED})
    message(STATUS "Exploding")
    execute_process(COMMAND ${JDK_HOME}/bin/jimage extract --dir "${JDK_EXPLODED}" "${JDK_HOME}/lib/modules")
endif ()

find_library(LIBJAVA java PATHS ${JDK_HOME}/lib REQUIRED)
#target_link_libraries(jvm ${LIBJAVA})

#
# Tests
#

# Note: using glob to create this list is not recommended
#file(GLOB JAVA_TEST_SOURCES CONFIGURE_DEPENDS tests/*.java)
set(JAVA_TEST_SOURCES
        tests/Annotations.java
        tests/ArrayCopy.java
        tests/Assertions.java
        tests/Bitwise.java
        tests/Boxing.java
        tests/Clazz.java
        tests/Collections.java
        tests/Empty.java
        tests/Exceptions.java
        # tests/ExceptionsConstruct.java
        tests/ExceptionsExit.java
        tests/ExceptionsExitNested.java
        tests/ExceptionsInheritance.java
        tests/Fields.java
        tests/HelloWorld.java
        tests/Initialization.java
        tests/InvokeStatic.java
        tests/Instanceof.java
        # tests/KarelTheRobot.java
        tests/Methods.java
        tests/MethodsStatic.java
        tests/MethodsVirtualInterface.java
        tests/MultiArrays.java
        tests/Native.java
        tests/ObjectArray.java
        tests/PropertiesTest.java
        tests/ReferenceComparisons.java
        tests/ReflectionTest.java
        tests/Strings.java
        tests/Switch.java
        tests/UnitBoolean.java
        )

set(JAVA_TEST_GENERATED_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticDoubleSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticFloatSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntShiftLeft.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntShiftRight.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntShiftRightU.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticIntSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongAdd.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongDiv.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongMul.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongNeg.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongRem.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongShiftLeft.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongShiftRight.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongShiftRightU.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/ArithmeticLongSub.java
        ${CMAKE_CURRENT_SOURCE_DIR}/tests-generated/out/Arrays.java
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
add_dependencies(jvm tests)

enable_testing()
# https://cmake.org/cmake/help/latest/command/add_test.html
# https://cmake.org/cmake/help/latest/guide/tutorial/index.html#id11
function(do_test path)
    get_filename_component(name ${path} NAME_WE)
    add_test(NAME ${name} COMMAND sh "${CMAKE_SOURCE_DIR}/compare.sh" ${Java_JAVA_EXECUTABLE} ${name} ${JDK_HOME})
endfunction(do_test)

foreach (source ${JAVA_TEST_SOURCES} ${JAVA_TEST_GENERATED_SOURCES})
    do_test(${source})
endforeach ()
