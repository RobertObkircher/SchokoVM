

#include "types.hpp"

typedef void JNIEnv;

#define JNIEXPORT __attribute__((visibility("default")))

#define JNICALL

typedef s4 jint;
typedef s8 jlong;
typedef s1 jbyte;
typedef u1 jboolean;
typedef u2 jchar;
typedef s2 jshort;
typedef float jfloat;
typedef double jdouble;
typedef jint jsize;

typedef void* jclass;
typedef void* jobject;

extern "C" {

JNIEXPORT jlong JNICALL
Java_Native_return42(JNIEnv *, jclass) {
    return 42;
}

JNIEXPORT jboolean JNICALL
Java_Native_returnTrue(JNIEnv *, jclass) {
    return true;
}

JNIEXPORT jlong JNICALL
Java_Native_returnId(JNIEnv *, jclass, jlong i) {
    return i;
}

JNIEXPORT jlong JNICALL
Java_Native_xor(JNIEnv *, jclass, jlong i, jlong j) {
    return i ^ j;
}

JNIEXPORT jboolean JNICALL
Java_Native_intsEqual(JNIEnv *, jclass, jint i, jint j) {
    return i == j;
}

JNIEXPORT jdouble JNICALL
Java_Native_plus(JNIEnv *, jclass, jdouble d, jfloat f) {
    return d + (double) f;
}

static jint value = 100;

JNIEXPORT void JNICALL
Java_Native_vvvvvvvvv(JNIEnv *, jclass) {
    ++value;
}

JNIEXPORT void JNICALL
Java_Native_setValue(JNIEnv *, jclass, jint v) {
    value = v;
}

JNIEXPORT jint JNICALL
Java_Native_getValue(JNIEnv *, jclass) {
    return value;
}

JNIEXPORT jdouble JNICALL
Java_Native_sum(JNIEnv *, jclass, jint i, jdouble d, jfloat f, jchar c, jlong l) {
    return (double) i + d + (double) f + (double) c + (double) l;
}

JNIEXPORT jint JNICALL
Java_Native_first(JNIEnv *, jobject, jint x, jint, jint) {
    return x;
}

JNIEXPORT jint JNICALL
Java_Native_second(JNIEnv *, jobject, jint, jint x, jint) {
    return x;
}

JNIEXPORT jint JNICALL
Java_Native_third(JNIEnv *, jobject, jint, jint, jint x) {
    return x;
}

}
