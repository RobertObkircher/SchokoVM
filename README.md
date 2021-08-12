# SchokoVM

[![Run tests](https://github.com/RobertObkircher/SchokoVM/actions/workflows/tests.yml/badge.svg)](https://github.com/RobertObkircher/SchokoVM/actions/workflows/tests.yml)

A java bytecode interpreter that was implemented by two students for the course on [Abstract Machines](https://www.complang.tuwien.ac.at/andi/185.966.html) at the TU Wien.

The name was inspired by [cacao jvm](http://www.cacaojvm.org/).

Slides of our presentation: [presentation.pdf](presentation.pdf)

Working:
- Load class and jar files.
- All instructions except invokedynamic.
- Objects/arrays.
- Exceptions.
- Native method calls.

Partially:
- OpenJDK 11 Class Library (System.out.println works).
- Some functions are still missing to use libjvm with `java -XXaltjvm=`.
- Primitive garbage collection.

Not implemented:
- Invokedynamic (String concatenation).
- Multi-threading / synchronized.
- Custom class loaders.
- Bytecode verification.

# Build and test

The repository contains run configurations for the CLion IDE.

Command line instructions:

```sh
mkdir build
cd build
cmake ..
make -j
ctest -j 12
```

Ctest uses the script [compare.sh](compare.sh) to compare SchokoVM to the system JDK.

# Dependencies

- Linux or macOS

- OpenJDK 11:

  If CMake finds the wrong version, you can set the JAVA_HOME environment variable.

- `libzip`, `libdl`, `libffi`

# OpenJDK 11 sources

You can optionally download the source code for OpenJDK 11 with this script:
`cd jdk && ./download.sh`. This is not necessary to compile the project.

# Notes

Some bullet points about the jvm specification can be found in [JvmSpecNotes.md](JvmSpecNotes.md).
