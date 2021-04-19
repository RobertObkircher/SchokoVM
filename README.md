# SchokoVM

A Java Bytecode interpreter. 

[![Run tests](https://github.com/RobertObkircher/SchokoVM/actions/workflows/tests.yml/badge.svg)](https://github.com/RobertObkircher/SchokoVM/actions/workflows/tests.yml)

# GDB

You need a ~/.gdbinit in your home directory or the project .gdbinit file will be ignored:

```
set auto-load local-gdbinit on 
add-auto-load-safe-path / 
```

The project `.gdbinit` file allows you to debug testcases that are executed by `compare.sh`.

# Notes

Some bullet points about the jvm specification can be found in [JvmSpecNotes.md](JvmSpecNotes.md).
