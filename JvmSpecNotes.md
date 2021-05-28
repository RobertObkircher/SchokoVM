http://www.javaseiten.de/jvmisi.html#iload_1
https://jvilk.com/blog/java-8-wtf-ambiguous-method-lookup/

# Chapter 2: The Structure of the Java Virtual Machine 

https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html

- char = unsigned utf16 codepoint
- type returnAddress: pointers to opcodes, used by jsr, ret, and jsr_w
- reference types: class, array, interface
- pc (program counter) register per thread
    - current jvm instruction if not in native method
    - can hold returnAddress
- jvm stack per thread
    - stores frames
    - memory does not need to be contiguous
    - never manipulated directly except to push and pop frames
- Heap (shared)
- method area (shared)
    - It stores per-class structures such as the run-time constant pool, field and method data, and the code for methods and constructors, including the special methods used in class and interface initialization and in instance initialization
    - logically part of the heap, simple implementations may choose not to either garbage collect or compact it. This specification does not mandate the location of the method area or the policies used to manage compiled code.
- run-time constant pool
    - per-class or per-interface run-time representation of the constant_pool table in a class file
    - ranging from numeric literals known at compile-time to method and field references that must be resolved at run-time
    - similar to symbol table for a conventional programming language
    - allocated from the method area
    - constructed when the class or interface is created
- native method stacks are typically allocated per thread when each thread is created
- frames:
    - its own local variables, operand stack, reference to run-time constant pool of the class of the current method
    - size does not change
    - to perform dynamic linking, return values for methods, and dispatch exceptions.
    - may be heap allocated
    - local to thread
- local variables
    - type long or type double occupies two consecutive local variables. Such a value may only be addressed using the lesser index.
    - parameters are passed in consecutive local variables starting from local variable 0 (for method 0 = "this")
- operand stack
    - max depth known at compile-time
    - entries can hold values of any type
    - depth, where a value of type long or double contributes two units to the depth and a value of any other type contributes one unit
- dynamic linking
    - Each frame contains a reference to the run-time constant pool for the type of the current method to support dynamic linking of the method code. The class file code for a method refers to methods to be invoked and variables to be accessed via symbolic references. Dynamic linking translates these symbolic method references into concrete method references, loading classes as necessary to resolve as-yet-undefined symbols, and translates variable accesses into appropriate offsets in storage structures associated with the run-time location of these variables.
- special methods
    - instance initialization methods (constructor)
        - void <init>()
        - invokespecial
    - class initialization methods
        - void <clinit>()
    - signature polymorphic methods
        - MethodHandle and Varhandle
        - dynamically strongly typed
- exceptions
    - when
        - athrow instruction
        - after execution of an instruction
        - asynchronous (at any time)
            - The stop method of class Thread or ThreadGroup was invoked
            - internal jvm error
    - exception handler
        - range where it is active
        - superclass<throwable
        - location of handler
- instruction set summary
    - big endian (byte1 << 8) | byte2
    - boolean, byte, char, and short are often treated as int

- the rest might be useful but it isn't worth summarizing


