#!/usr/bin/env zsh

# Don't forget to disable sanitizers when building SchokoVM for benchmarking

javac ../FibonacciRec.java -d .
time java FibonacciRec 36 1> /dev/null
time java -XX:-UseCompiler FibonacciRec 36 1> /dev/null
time ./SchokoVM --java-home $JAVA_HOME FibonacciRec 36 1> /dev/null

echo

javac ../FibonacciIter.java -d .
time java FibonacciIter 100000 1> /dev/null
time java -XX:-UseCompiler FibonacciIter 100000 1> /dev/null
time ./SchokoVM --java-home $JAVA_HOME FibonacciIter 100000 1> /dev/null

echo

javac ../FibonacciIterBig.java -d .
time java FibonacciIterBig 100000 1> /dev/null
time java -XX:-UseCompiler FibonacciIterBig 100000 1> /dev/null
time ./SchokoVM --java-home $JAVA_HOME FibonacciIterBig 100000 1> /dev/null
