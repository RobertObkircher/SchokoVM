#ifndef SCHOKOVM_UNSAFE_HPP
#define SCHOKOVM_UNSAFE_HPP

#include "jni.h"

extern "C" {
_JNI_IMPORT_OR_EXPORT_
void JNICALL Java_jdk_internal_misc_Unsafe_registerNatives(JNIEnv *env, jclass clazz);
}

#endif //SCHOKOVM_UNSAFE_HPP
