#include "jni.h"
#include <vector>

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

// NOLINTNEXTLINE
JNIEXPORT jint JNICALL Java_Native_overloaded__I(JNIEnv *, jclass, jint a) {
    return a;
}
// NOLINTNEXTLINE
JNIEXPORT jint JNICALL Java_Native_overloaded__J(JNIEnv *, jclass, jlong b) {
    return static_cast<jint>(b);
}
// NOLINTNEXTLINE
JNIEXPORT jstring JNICALL Java_Native_overloaded__Ljava_lang_String_2(JNIEnv *env, jclass, jstring) {
    return env->NewStringUTF("abc");
}
// NOLINTNEXTLINE
JNIEXPORT jint JNICALL Java_Native_overloaded___3I(JNIEnv *env, jclass, jintArray arr) {
    return env->GetArrayLength(arr);
}

JNIEXPORT jbyteArray JNICALL Java_Native_strings(JNIEnv *env, jclass, jstring str, jsize start, jsize region_len) {
    std::vector<jint> result;

    jsize len = env->GetStringLength(str);
    result.push_back(len);

    jchar const *chars = env->GetStringChars(str, nullptr);
    for (jint i = 0; i < len; ++i) {
        result.push_back(chars[i]);
    }
    env->ReleaseStringChars(str, chars);

    std::vector<jchar> region;
    region.resize(static_cast<size_t>(region_len));
    env->GetStringRegion(str, start, region_len, region.data());
    for (jchar c : region) {
        result.push_back(c);
    }

    auto return_size = static_cast<jsize>(sizeof(jint) * result.size());
    jbyteArray return_value = env->NewByteArray(return_size);
    env->SetByteArrayRegion(return_value, 0, return_size, reinterpret_cast<const jbyte *>(result.data()));
    return return_value;
}

JNIEXPORT jbyteArray JNICALL Java_Native_strings8(JNIEnv *env, jclass, jstring str, jsize start, jsize region_len) {
    std::vector<jint> result;

    jsize len = env->GetStringUTFLength(str);
    result.push_back(len);

    char const *chars = env->GetStringUTFChars(str, nullptr);
    for (jint i = 0; i < len + 1; ++i) { // alos check for 0 termination
        result.push_back(chars[i]);
    }
    env->ReleaseStringUTFChars(str, chars);

    std::vector<char> region;
    region.resize(static_cast<size_t>(len) + 1); // we don't know the exact length but also check for null termincation
    env->GetStringUTFRegion(str, start, region_len, region.data());
    for (char c : region) {
        result.push_back(c);
    }

    auto return_size = static_cast<jsize>(sizeof(jint) * result.size());
    jbyteArray return_value = env->NewByteArray(return_size);
    env->SetByteArrayRegion(return_value, 0, return_size, reinterpret_cast<const jbyte *>(result.data()));
    return return_value;
}

}

