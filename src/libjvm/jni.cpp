#include <cstdlib>

#include "jni.h"

#define UNIMPLEMENTED(name) do {           \
    printf("UNIMPLEMENTED: %s\n", name);   \
} while(0);

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_GetDefaultJavaVMInitArgs(void *args) {
    UNIMPLEMENTED("JNI_GetDefaultJavaVMInitArgs");
    return 0;
}

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_CreateJavaVM(JavaVM **pvm, void **penv, void *args) {
    UNIMPLEMENTED("JNI_CreateJavaVM");
    return 0;
}

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_GetCreatedJavaVMs(JavaVM **, jsize, jsize *) {
    UNIMPLEMENTED("JNI_GetCreatedJavaVMs");
    return 0;
}

/* Defined by native libraries. */
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    UNIMPLEMENTED("JNI_OnLoad");
    return 0;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *vm, void *reserved) { UNIMPLEMENTED("JNI_OnUnload"); }

