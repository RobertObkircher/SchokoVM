#include "jni.h"

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

JNIEXPORT jint JNICALL
Java_Native_summm(JNIEnv *, jclass,
                  jint a1, jint a2, jint a3, jint a4, jint a5, jint a6, jint a7, jint a8, jint a9, jint a10,
                  jint a11, jint a12, jint a13, jint a14, jint a15, jint a16, jint a17, jint a18, jint a19) {
    return a1 + +a2 + +a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19;
}

}

