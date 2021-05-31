# SchokoVM

A Java Bytecode interpreter. 

[![Run tests](https://github.com/RobertObkircher/SchokoVM/actions/workflows/tests.yml/badge.svg)](https://github.com/RobertObkircher/SchokoVM/actions/workflows/tests.yml)

# OpenJDK

You need to download an OpenJDK release and the sources: `cd jdk && ./download.sh`

# GDB

You need a ~/.gdbinit in your home directory or the project .gdbinit file will be ignored:

```
set auto-load local-gdbinit on 
add-auto-load-safe-path / 
```

The project `.gdbinit` file allows you to debug testcases that are executed by `compare.sh`.

# OpenJDK rt.jar

Since java 9 OpenJDK uses the jrt filesystem to store the class library.
I tried the following to get something like the old rt.jar.

```sh
jimage extract --dir modules /usr/lib/jvm/default/lib/modules
mkdir rt
cp -fr modules/*/* rt
cd rt
zip -r ../rt.jar .
rm -rf modules rt
```

# Notes

Some bullet points about the jvm specification can be found in [JvmSpecNotes.md](JvmSpecNotes.md).
