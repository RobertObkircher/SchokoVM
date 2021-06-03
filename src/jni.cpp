#include <iostream>
#include <vector>

#define _JNI_IMPLEMENTATION_

#include "jni.h"
#include "classloading.hpp"

#define UNIMPLEMENTED(x) std::cerr << x; exit(42);

/**
 * The Invocation API
 *
 * https://docs.oracle.com/en/java/javase/11/docs/specs/jni/invocation.html
 */

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_GetDefaultJavaVMInitArgs(void *args) {
    printf("JNI_GetDefaultJavaVMInitArgs\n");
    return 0;
}

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_CreateJavaVM(JavaVM **pvm, void **penv, void *args) {
    printf("JNI_CreateJavaVM\n");
    return 0;
}

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_GetCreatedJavaVMs(JavaVM **vmBuf, jsize bufLen, jsize *nVMs) {
    UNIMPLEMENTED("JNI_GetCreatedJavaVMs");
}

/**
 * Functions for struct JNIInvokeInterface_
 */

jint JNICALL
DestroyJavaVM(JavaVM *vm) {
    UNIMPLEMENTED("DestroyJavaVM");
}

jint JNICALL
AttachCurrentThread(JavaVM *vm, void **penv, void *args) {
    UNIMPLEMENTED("AttachCurrentThread");
}

jint JNICALL
DetachCurrentThread(JavaVM *vm) {
    UNIMPLEMENTED("DetachCurrentThread");
}

jint JNICALL
GetEnv(JavaVM *vm, void **penv, jint version) {
    UNIMPLEMENTED("GetEnv");
}

jint JNICALL
AttachCurrentThreadAsDaemon(JavaVM *vm, void **penv, void *args) {
    UNIMPLEMENTED("AttachCurrentThreadAsDaemon");
}

const struct JNIInvokeInterface_ jni_invoke_interface{
        nullptr,
        nullptr,
        nullptr,
        DestroyJavaVM,
        AttachCurrentThread,
        DetachCurrentThread,
        GetEnv,
        AttachCurrentThreadAsDaemon,
};

/**
 * Functions for struct JNINativeInterface_
 */

jint GetVersion(JNIEnv *env) {
    UNIMPLEMENTED("GetVersion");
}

jclass DefineClass
        (JNIEnv *env, const char *name, jobject loader, const jbyte *buf,
         jsize len) {
    UNIMPLEMENTED("DefineClass");
}

jclass FindClass
        (JNIEnv *env, const char *name) {
    UNIMPLEMENTED("FindClass");
}

jmethodID FromReflectedMethod
        (JNIEnv *env, jobject method) {
    UNIMPLEMENTED("FromReflectedMethod");
}

jfieldID FromReflectedField
        (JNIEnv *env, jobject field) {
    UNIMPLEMENTED("FromReflectedField");
}

jobject ToReflectedMethod
        (JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic) {
    UNIMPLEMENTED("ToReflectedMethod");
}

jclass GetSuperclass
        (JNIEnv *env, jclass sub) {
    UNIMPLEMENTED("GetSuperclass");
}

jboolean IsAssignableFrom
        (JNIEnv *env, jclass sub, jclass sup) {
    UNIMPLEMENTED("IsAssignableFrom");
}

jobject ToReflectedField
        (JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic) {
    UNIMPLEMENTED("ToReflectedField");
}

jint Throw
        (JNIEnv *env, jthrowable obj) {
    UNIMPLEMENTED("Throw");
}

jint ThrowNew
        (JNIEnv *env, jclass clazz, const char *msg) {
    UNIMPLEMENTED("ThrowNew");
}

jthrowable ExceptionOccurred
        (JNIEnv *env) {
    UNIMPLEMENTED("ExceptionOccurred");
}

void ExceptionDescribe
        (JNIEnv *env) {
    UNIMPLEMENTED("ExceptionDescribe");
}

void ExceptionClear
        (JNIEnv *env) {
    UNIMPLEMENTED("ExceptionClear");
}

void FatalError
        (JNIEnv *env, const char *msg) {
    UNIMPLEMENTED("FatalError");
}

jint PushLocalFrame
        (JNIEnv *env, jint capacity) {
    UNIMPLEMENTED("PushLocalFrame");
}

jobject PopLocalFrame
        (JNIEnv *env, jobject result) {
    UNIMPLEMENTED("PopLocalFrame");
}

jobject NewGlobalRef
        (JNIEnv *env, jobject lobj) {
    UNIMPLEMENTED("NewGlobalRef");
}

void DeleteGlobalRef
        (JNIEnv *env, jobject gref) {
    UNIMPLEMENTED("DeleteGlobalRef");
}

void DeleteLocalRef
        (JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("DeleteLocalRef");
}

jboolean IsSameObject
        (JNIEnv *env, jobject obj1, jobject obj2) {
    UNIMPLEMENTED("IsSameObject");
}

jobject NewLocalRef
        (JNIEnv *env, jobject ref) {
    UNIMPLEMENTED("NewLocalRef");
}

jint EnsureLocalCapacity
        (JNIEnv *env, jint capacity) {
    UNIMPLEMENTED("EnsureLocalCapacity");
}

jobject AllocObject
        (JNIEnv *env, jclass clazz) {
    UNIMPLEMENTED("AllocObject");
}

jobject NewObject
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("NewObject");
}

jobject NewObjectV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("NewObjectV");
}

jobject NewObjectA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("NewObjectA");
}

jclass GetObjectClass
        (JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("GetObjectClass");
}

jboolean IsInstanceOf
        (JNIEnv *env, jobject obj, jclass clazz) {
    UNIMPLEMENTED("IsInstanceOf");
}

jmethodID GetMethodID
        (JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    UNIMPLEMENTED("GetMethodID");
}

jobject CallObjectMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallObjectMethod");
}

jobject CallObjectMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallObjectMethodV");
}

jobject CallObjectMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallObjectMethodA");
}

jboolean CallBooleanMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallBooleanMethod");
}

jboolean CallBooleanMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallBooleanMethodV");
}

jboolean CallBooleanMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallBooleanMethodA");
}

jbyte CallByteMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallByteMethod");
}

jbyte CallByteMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallByteMethodV");
}

jbyte CallByteMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallByteMethodA");
}

jchar CallCharMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallCharMethod");
}

jchar CallCharMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallCharMethodV");
}

jchar CallCharMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallCharMethodA");
}

jshort CallShortMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallShortMethod");
}

jshort CallShortMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallShortMethodV");
}

jshort CallShortMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallShortMethodA");
}

jint CallIntMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallIntMethod");
}

jint CallIntMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallIntMethodV");
}

jint CallIntMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallIntMethodA");
}

jlong CallLongMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallLongMethod");
}

jlong CallLongMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallLongMethodV");
}

jlong CallLongMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallLongMethodA");
}

jfloat CallFloatMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallFloatMethod");
}

jfloat CallFloatMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallFloatMethodV");
}

jfloat CallFloatMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallFloatMethodA");
}

jdouble CallDoubleMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallDoubleMethod");
}

jdouble CallDoubleMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallDoubleMethodV");
}

jdouble CallDoubleMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallDoubleMethodA");
}

void CallVoidMethod
        (JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallVoidMethod");
}

void CallVoidMethodV
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallVoidMethodV");
}

void CallVoidMethodA
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallVoidMethodA");
}

jobject CallNonvirtualObjectMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualObjectMethod");
}

jobject CallNonvirtualObjectMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualObjectMethodV");
}

jobject CallNonvirtualObjectMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualObjectMethodA");
}

jboolean CallNonvirtualBooleanMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualBooleanMethod");
}

jboolean CallNonvirtualBooleanMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualBooleanMethodV");
}

jboolean CallNonvirtualBooleanMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualBooleanMethodA");
}

jbyte CallNonvirtualByteMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualByteMethod");
}

jbyte CallNonvirtualByteMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualByteMethodV");
}

jbyte CallNonvirtualByteMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualByteMethodA");
}

jchar CallNonvirtualCharMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualCharMethod");
}

jchar CallNonvirtualCharMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualCharMethodV");
}

jchar CallNonvirtualCharMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualCharMethodA");
}

jshort CallNonvirtualShortMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualShortMethod");
}

jshort CallNonvirtualShortMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualShortMethodV");
}

jshort CallNonvirtualShortMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualShortMethodA");
}

jint CallNonvirtualIntMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualIntMethod");
}

jint CallNonvirtualIntMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualIntMethodV");
}

jint CallNonvirtualIntMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualIntMethodA");
}

jlong CallNonvirtualLongMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualLongMethod");
}

jlong CallNonvirtualLongMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualLongMethodV");
}

jlong CallNonvirtualLongMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualLongMethodA");
}

jfloat CallNonvirtualFloatMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualFloatMethod");
}

jfloat CallNonvirtualFloatMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualFloatMethodV");
}

jfloat CallNonvirtualFloatMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualFloatMethodA");
}

jdouble CallNonvirtualDoubleMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualDoubleMethod");
}

jdouble CallNonvirtualDoubleMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualDoubleMethodV");
}

jdouble CallNonvirtualDoubleMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualDoubleMethodA");
}

void CallNonvirtualVoidMethod
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallNonvirtualVoidMethod");
}

void CallNonvirtualVoidMethodV
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         va_list args) {
    UNIMPLEMENTED("CallNonvirtualVoidMethodV");
}

void CallNonvirtualVoidMethodA
        (JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID,
         const jvalue *args) {
    UNIMPLEMENTED("CallNonvirtualVoidMethodA");
}

jfieldID GetFieldID
        (JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    UNIMPLEMENTED("GetFieldID");
}

jobject GetObjectField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetObjectField");
}

jboolean GetBooleanField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetBooleanField");
}

jbyte GetByteField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetByteField");
}

jchar GetCharField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetCharField");
}

jshort GetShortField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetShortField");
}

jint GetIntField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetIntField");
}

jlong GetLongField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetLongField");
}

jfloat GetFloatField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetFloatField");
}

jdouble GetDoubleField
        (JNIEnv *env, jobject obj, jfieldID fieldID) {
    UNIMPLEMENTED("GetDoubleField");
}

void SetObjectField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jobject val) {
    UNIMPLEMENTED("SetObjectField");
}

void SetBooleanField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jboolean val) {
    UNIMPLEMENTED("SetBooleanField");
}

void SetByteField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jbyte val) {
    UNIMPLEMENTED("SetByteField");
}

void SetCharField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jchar val) {
    UNIMPLEMENTED("SetCharField");
}

void SetShortField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jshort val) {
    UNIMPLEMENTED("SetShortField");
}

void SetIntField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jint val) {
    UNIMPLEMENTED("SetIntField");
}

void SetLongField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jlong val) {
    UNIMPLEMENTED("SetLongField");
}

void SetFloatField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jfloat val) {
    UNIMPLEMENTED("SetFloatField");
}

void SetDoubleField
        (JNIEnv *env, jobject obj, jfieldID fieldID, jdouble val) {
    UNIMPLEMENTED("SetDoubleField");
}

jmethodID GetStaticMethodID
        (JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    UNIMPLEMENTED("GetStaticMethodID");
}

jobject CallStaticObjectMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticObjectMethod");
}

jobject CallStaticObjectMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticObjectMethodV");
}

jobject CallStaticObjectMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticObjectMethodA");
}

jboolean CallStaticBooleanMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticBooleanMethod");
}

jboolean CallStaticBooleanMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticBooleanMethodV");
}

jboolean CallStaticBooleanMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticBooleanMethodA");
}

jbyte CallStaticByteMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticByteMethod");
}

jbyte CallStaticByteMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticByteMethodV");
}

jbyte CallStaticByteMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticByteMethodA");
}

jchar CallStaticCharMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticCharMethod");
}

jchar CallStaticCharMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticCharMethodV");
}

jchar CallStaticCharMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticCharMethodA");
}

jshort CallStaticShortMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticShortMethod");
}

jshort CallStaticShortMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticShortMethodV");
}

jshort CallStaticShortMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticShortMethodA");
}

jint CallStaticIntMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticIntMethod");
}

jint CallStaticIntMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticIntMethodV");
}

jint CallStaticIntMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticIntMethodA");
}

jlong CallStaticLongMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticLongMethod");
}

jlong CallStaticLongMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticLongMethodV");
}

jlong CallStaticLongMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticLongMethodA");
}

jfloat CallStaticFloatMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticFloatMethod");
}

jfloat CallStaticFloatMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticFloatMethodV");
}

jfloat CallStaticFloatMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticFloatMethodA");
}

jdouble CallStaticDoubleMethod
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticDoubleMethod");
}

jdouble CallStaticDoubleMethodV
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticDoubleMethodV");
}

jdouble CallStaticDoubleMethodA
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticDoubleMethodA");
}

void CallStaticVoidMethod
        (JNIEnv *env, jclass cls, jmethodID methodID, ...) {
    UNIMPLEMENTED("CallStaticVoidMethod");
}

void CallStaticVoidMethodV
        (JNIEnv *env, jclass cls, jmethodID methodID, va_list args) {
    UNIMPLEMENTED("CallStaticVoidMethodV");
}

void CallStaticVoidMethodA
        (JNIEnv *env, jclass cls, jmethodID methodID, const jvalue *args) {
    UNIMPLEMENTED("CallStaticVoidMethodA");
}

jfieldID GetStaticFieldID
        (JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    UNIMPLEMENTED("GetStaticFieldID");
}

jobject GetStaticObjectField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticObjectField");
}

jboolean GetStaticBooleanField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticBooleanField");
}

jbyte GetStaticByteField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticByteField");
}

jchar GetStaticCharField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticCharField");
}

jshort GetStaticShortField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticShortField");
}

jint GetStaticIntField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticIntField");
}

jlong GetStaticLongField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticLongField");
}

jfloat GetStaticFloatField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticFloatField");
}

jdouble GetStaticDoubleField
        (JNIEnv *env, jclass clazz, jfieldID fieldID) {
    UNIMPLEMENTED("GetStaticDoubleField");
}

void SetStaticObjectField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value) {
    UNIMPLEMENTED("SetStaticObjectField");
}

void SetStaticBooleanField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value) {
    UNIMPLEMENTED("SetStaticBooleanField");
}

void SetStaticByteField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value) {
    UNIMPLEMENTED("SetStaticByteField");
}

void SetStaticCharField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value) {
    UNIMPLEMENTED("SetStaticCharField");
}

void SetStaticShortField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value) {
    UNIMPLEMENTED("SetStaticShortField");
}

void SetStaticIntField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jint value) {
    UNIMPLEMENTED("SetStaticIntField");
}

void SetStaticLongField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value) {
    UNIMPLEMENTED("SetStaticLongField");
}

void SetStaticFloatField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value) {
    UNIMPLEMENTED("SetStaticFloatField");
}

void SetStaticDoubleField
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value) {
    UNIMPLEMENTED("SetStaticDoubleField");
}

jstring NewString
        (JNIEnv *env, const jchar *unicode, jsize len) {
    UNIMPLEMENTED("NewString");
}

jsize GetStringLength
        (JNIEnv *env, jstring str) {
    UNIMPLEMENTED("GetStringLength");
}

const jchar *GetStringChars
        (JNIEnv *env, jstring str, jboolean *isCopy) {
    UNIMPLEMENTED("GetStringChars");
}

void ReleaseStringChars
        (JNIEnv *env, jstring str, const jchar *chars) {
    UNIMPLEMENTED("ReleaseStringChars");
}

jstring NewStringUTF
        (JNIEnv *env, const char *utf) {
    UNIMPLEMENTED("NewStringUTF");
}

jsize GetStringUTFLength
        (JNIEnv *env, jstring str) {
    UNIMPLEMENTED("GetStringUTFLength");
}

const char *GetStringUTFChars
        (JNIEnv *env, jstring str, jboolean *isCopy) {
    UNIMPLEMENTED("GetStringUTFChars");
}

void ReleaseStringUTFChars
        (JNIEnv *env, jstring str, const char *chars) {
    UNIMPLEMENTED("ReleaseStringUTFChars");
}


jsize GetArrayLength
        (JNIEnv *env, jarray array) {
    UNIMPLEMENTED("GetArrayLength");
}

jobjectArray NewObjectArray
        (JNIEnv *env, jsize len, jclass clazz, jobject init) {
    UNIMPLEMENTED("NewObjectArray");
}

jobject GetObjectArrayElement
        (JNIEnv *env, jobjectArray array, jsize index) {
    UNIMPLEMENTED("GetObjectArrayElement");
}

void SetObjectArrayElement
        (JNIEnv *env, jobjectArray array, jsize index, jobject val) {
    UNIMPLEMENTED("SetObjectArrayElement");
}

jbooleanArray NewBooleanArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewBooleanArray");
}

jbyteArray NewByteArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewByteArray");
}

jcharArray NewCharArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewCharArray");
}

jshortArray NewShortArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewShortArray");
}

jintArray NewIntArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewIntArray");
}

jlongArray NewLongArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewLongArray");
}

jfloatArray NewFloatArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewFloatArray");
}

jdoubleArray NewDoubleArray
        (JNIEnv *env, jsize len) {
    UNIMPLEMENTED("NewDoubleArray");
}

jboolean *GetBooleanArrayElements
        (JNIEnv *env, jbooleanArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetBooleanArrayElements");
}

jbyte *GetByteArrayElements
        (JNIEnv *env, jbyteArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetByteArrayElements");
}

jchar *GetCharArrayElements
        (JNIEnv *env, jcharArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetCharArrayElements");
}

jshort *GetShortArrayElements
        (JNIEnv *env, jshortArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetShortArrayElements");
}

jint *GetIntArrayElements
        (JNIEnv *env, jintArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetIntArrayElements");
}

jlong *GetLongArrayElements
        (JNIEnv *env, jlongArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetLongArrayElements");
}

jfloat *GetFloatArrayElements
        (JNIEnv *env, jfloatArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetFloatArrayElements");
}

jdouble *GetDoubleArrayElements
        (JNIEnv *env, jdoubleArray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetDoubleArrayElements");
}

void ReleaseBooleanArrayElements
        (JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode) {
    UNIMPLEMENTED("ReleaseBooleanArrayElements");
}

void ReleaseByteArrayElements
        (JNIEnv *env, jbyteArray array, jbyte *elems, jint mode) {
    UNIMPLEMENTED("ReleaseByteArrayElements");
}

void ReleaseCharArrayElements
        (JNIEnv *env, jcharArray array, jchar *elems, jint mode) {
    UNIMPLEMENTED("ReleaseCharArrayElements");
}

void ReleaseShortArrayElements
        (JNIEnv *env, jshortArray array, jshort *elems, jint mode) {
    UNIMPLEMENTED("ReleaseShortArrayElements");
}

void ReleaseIntArrayElements
        (JNIEnv *env, jintArray array, jint *elems, jint mode) {
    UNIMPLEMENTED("ReleaseIntArrayElements");
}

void ReleaseLongArrayElements
        (JNIEnv *env, jlongArray array, jlong *elems, jint mode) {
    UNIMPLEMENTED("ReleaseLongArrayElements");
}

void ReleaseFloatArrayElements
        (JNIEnv *env, jfloatArray array, jfloat *elems, jint mode) {
    UNIMPLEMENTED("ReleaseFloatArrayElements");
}

void ReleaseDoubleArrayElements
        (JNIEnv *env, jdoubleArray array, jdouble *elems, jint mode) {
    UNIMPLEMENTED("ReleaseDoubleArrayElements");
}

void GetBooleanArrayRegion
        (JNIEnv *env, jbooleanArray array, jsize start, jsize l, jboolean *buf) {
    UNIMPLEMENTED("GetBooleanArrayRegion");
}

void GetByteArrayRegion
        (JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf) {
    UNIMPLEMENTED("GetByteArrayRegion");
}

void GetCharArrayRegion
        (JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf) {
    UNIMPLEMENTED("GetCharArrayRegion");
}

void GetShortArrayRegion
        (JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf) {
    UNIMPLEMENTED("GetShortArrayRegion");
}

void GetIntArrayRegion
        (JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf) {
    UNIMPLEMENTED("GetIntArrayRegion");
}

void GetLongArrayRegion
        (JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf) {
    UNIMPLEMENTED("GetLongArrayRegion");
}

void GetFloatArrayRegion
        (JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf) {
    UNIMPLEMENTED("GetFloatArrayRegion");
}

void GetDoubleArrayRegion
        (JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf) {
    UNIMPLEMENTED("GetDoubleArrayRegion");
}

void SetBooleanArrayRegion
        (JNIEnv *env, jbooleanArray array, jsize start, jsize l, const jboolean *buf) {
    UNIMPLEMENTED("SetBooleanArrayRegion");
}

void SetByteArrayRegion
        (JNIEnv *env, jbyteArray array, jsize start, jsize len, const jbyte *buf) {
    UNIMPLEMENTED("SetByteArrayRegion");
}

void SetCharArrayRegion
        (JNIEnv *env, jcharArray array, jsize start, jsize len, const jchar *buf) {
    UNIMPLEMENTED("SetCharArrayRegion");
}

void SetShortArrayRegion
        (JNIEnv *env, jshortArray array, jsize start, jsize len, const jshort *buf) {
    UNIMPLEMENTED("SetShortArrayRegion");
}

void SetIntArrayRegion
        (JNIEnv *env, jintArray array, jsize start, jsize len, const jint *buf) {
    UNIMPLEMENTED("SetIntArrayRegion");
}

void SetLongArrayRegion
        (JNIEnv *env, jlongArray array, jsize start, jsize len, const jlong *buf) {
    UNIMPLEMENTED("SetLongArrayRegion");
}

void SetFloatArrayRegion
        (JNIEnv *env, jfloatArray array, jsize start, jsize len, const jfloat *buf) {
    UNIMPLEMENTED("SetFloatArrayRegion");
}

void SetDoubleArrayRegion
        (JNIEnv *env, jdoubleArray array, jsize start, jsize len, const jdouble *buf) {
    UNIMPLEMENTED("SetDoubleArrayRegion");
}

jint RegisterNatives
        (JNIEnv *env, jclass clazz, const JNINativeMethod *methods,
         jint nMethods) {
    UNIMPLEMENTED("RegisterNatives");
}

jint UnregisterNatives
        (JNIEnv *env, jclass clazz) {
    UNIMPLEMENTED("UnregisterNatives");
}

jint MonitorEnter
        (JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("MonitorEnter");
}

jint MonitorExit
        (JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("MonitorExit");
}

jint GetJavaVM
        (JNIEnv *env, JavaVM **vm) {
    UNIMPLEMENTED("GetJavaVM");
}

void GetStringRegion
        (JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf) {
    UNIMPLEMENTED("GetStringRegion");
}

void GetStringUTFRegion
        (JNIEnv *env, jstring str, jsize start, jsize len, char *buf) {
    UNIMPLEMENTED("GetStringUTFRegion");
}

void *GetPrimitiveArrayCritical
        (JNIEnv *env, jarray array, jboolean *isCopy) {
    UNIMPLEMENTED("GetPrimitiveArrayCritical");
}

void ReleasePrimitiveArrayCritical
        (JNIEnv *env, jarray array, void *carray, jint mode) {
    UNIMPLEMENTED("ReleasePrimitiveArrayCritical");
}

const jchar *GetStringCritical
        (JNIEnv *env, jstring string, jboolean *isCopy) {
    UNIMPLEMENTED("GetStringCritical");
}

void ReleaseStringCritical
        (JNIEnv *env, jstring string, const jchar *cstring) {
    UNIMPLEMENTED("ReleaseStringCritical");
}

jweak NewWeakGlobalRef
        (JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("NewWeakGlobalRef");
}

void DeleteWeakGlobalRef
        (JNIEnv *env, jweak ref) {
    UNIMPLEMENTED("DeleteWeakGlobalRef");
}

jboolean ExceptionCheck
        (JNIEnv *env) {
    UNIMPLEMENTED("ExceptionCheck");
}

jobject NewDirectByteBuffer
        (JNIEnv *env, void *address, jlong capacity) {
    UNIMPLEMENTED("NewDirectByteBuffer");
}

void *GetDirectBufferAddress
        (JNIEnv *env, jobject buf) {
    UNIMPLEMENTED("GetDirectBufferAddress");
}

jlong GetDirectBufferCapacity
        (JNIEnv *env, jobject buf) {
    UNIMPLEMENTED("GetDirectBufferCapacity");
}

/* New JNI 1.6 Features */

jobjectRefType GetObjectRefType
        (JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("GetObjectRefType");
}

/* Module Features */

jobject GetModule
        (JNIEnv *env, jclass clazz) {
    UNIMPLEMENTED("GetModule");
}

const struct JNINativeInterface_ jni_native_interface{
        nullptr,
        nullptr,
        nullptr,

        nullptr,
        GetVersion,

        DefineClass,
        FindClass,

        FromReflectedMethod,
        FromReflectedField,

        ToReflectedMethod,

        GetSuperclass,
        IsAssignableFrom,

        ToReflectedField,

        Throw,
        ThrowNew,
        ExceptionOccurred,
        ExceptionDescribe,
        ExceptionClear,
        FatalError,

        PushLocalFrame,
        PopLocalFrame,

        NewGlobalRef,
        DeleteGlobalRef,
        DeleteLocalRef,
        IsSameObject,
        NewLocalRef,
        EnsureLocalCapacity,

        AllocObject,
        NewObject,
        NewObjectV,
        NewObjectA,

        GetObjectClass,
        IsInstanceOf,

        GetMethodID,

        CallObjectMethod,
        CallObjectMethodV,
        CallObjectMethodA,

        CallBooleanMethod,
        CallBooleanMethodV,
        CallBooleanMethodA,

        CallByteMethod,
        CallByteMethodV,
        CallByteMethodA,

        CallCharMethod,
        CallCharMethodV,
        CallCharMethodA,

        CallShortMethod,
        CallShortMethodV,
        CallShortMethodA,

        CallIntMethod,
        CallIntMethodV,
        CallIntMethodA,

        CallLongMethod,
        CallLongMethodV,
        CallLongMethodA,

        CallFloatMethod,
        CallFloatMethodV,
        CallFloatMethodA,

        CallDoubleMethod,
        CallDoubleMethodV,
        CallDoubleMethodA,

        CallVoidMethod,
        CallVoidMethodV,
        CallVoidMethodA,

        CallNonvirtualObjectMethod,
        CallNonvirtualObjectMethodV,
        CallNonvirtualObjectMethodA,

        CallNonvirtualBooleanMethod,
        CallNonvirtualBooleanMethodV,
        CallNonvirtualBooleanMethodA,

        CallNonvirtualByteMethod,
        CallNonvirtualByteMethodV,
        CallNonvirtualByteMethodA,

        CallNonvirtualCharMethod,
        CallNonvirtualCharMethodV,
        CallNonvirtualCharMethodA,

        CallNonvirtualShortMethod,
        CallNonvirtualShortMethodV,
        CallNonvirtualShortMethodA,

        CallNonvirtualIntMethod,
        CallNonvirtualIntMethodV,
        CallNonvirtualIntMethodA,

        CallNonvirtualLongMethod,
        CallNonvirtualLongMethodV,
        CallNonvirtualLongMethodA,

        CallNonvirtualFloatMethod,
        CallNonvirtualFloatMethodV,
        CallNonvirtualFloatMethodA,

        CallNonvirtualDoubleMethod,
        CallNonvirtualDoubleMethodV,
        CallNonvirtualDoubleMethodA,

        CallNonvirtualVoidMethod,
        CallNonvirtualVoidMethodV,
        CallNonvirtualVoidMethodA,

        GetFieldID,

        GetObjectField,
        GetBooleanField,
        GetByteField,
        GetCharField,
        GetShortField,
        GetIntField,
        GetLongField,
        GetFloatField,
        GetDoubleField,

        SetObjectField,
        SetBooleanField,
        SetByteField,
        SetCharField,
        SetShortField,
        SetIntField,
        SetLongField,
        SetFloatField,
        SetDoubleField,

        GetStaticMethodID,

        CallStaticObjectMethod,
        CallStaticObjectMethodV,
        CallStaticObjectMethodA,

        CallStaticBooleanMethod,
        CallStaticBooleanMethodV,
        CallStaticBooleanMethodA,

        CallStaticByteMethod,
        CallStaticByteMethodV,
        CallStaticByteMethodA,

        CallStaticCharMethod,
        CallStaticCharMethodV,
        CallStaticCharMethodA,

        CallStaticShortMethod,
        CallStaticShortMethodV,
        CallStaticShortMethodA,

        CallStaticIntMethod,
        CallStaticIntMethodV,
        CallStaticIntMethodA,

        CallStaticLongMethod,
        CallStaticLongMethodV,
        CallStaticLongMethodA,

        CallStaticFloatMethod,
        CallStaticFloatMethodV,
        CallStaticFloatMethodA,

        CallStaticDoubleMethod,
        CallStaticDoubleMethodV,
        CallStaticDoubleMethodA,

        CallStaticVoidMethod,
        CallStaticVoidMethodV,
        CallStaticVoidMethodA,

        GetStaticFieldID,
        GetStaticObjectField,
        GetStaticBooleanField,
        GetStaticByteField,
        GetStaticCharField,
        GetStaticShortField,
        GetStaticIntField,
        GetStaticLongField,
        GetStaticFloatField,
        GetStaticDoubleField,

        SetStaticObjectField,
        SetStaticBooleanField,
        SetStaticByteField,
        SetStaticCharField,
        SetStaticShortField,
        SetStaticIntField,
        SetStaticLongField,
        SetStaticFloatField,
        SetStaticDoubleField,

        NewString,
        GetStringLength,
        GetStringChars,
        ReleaseStringChars,

        NewStringUTF,
        GetStringUTFLength,
        GetStringUTFChars,
        ReleaseStringUTFChars,


        GetArrayLength,

        NewObjectArray,
        GetObjectArrayElement,
        SetObjectArrayElement,

        NewBooleanArray,
        NewByteArray,
        NewCharArray,
        NewShortArray,
        NewIntArray,
        NewLongArray,
        NewFloatArray,
        NewDoubleArray,

        GetBooleanArrayElements,
        GetByteArrayElements,
        GetCharArrayElements,
        GetShortArrayElements,
        GetIntArrayElements,
        GetLongArrayElements,
        GetFloatArrayElements,
        GetDoubleArrayElements,

        ReleaseBooleanArrayElements,
        ReleaseByteArrayElements,
        ReleaseCharArrayElements,
        ReleaseShortArrayElements,
        ReleaseIntArrayElements,
        ReleaseLongArrayElements,
        ReleaseFloatArrayElements,
        ReleaseDoubleArrayElements,

        GetBooleanArrayRegion,
        GetByteArrayRegion,
        GetCharArrayRegion,
        GetShortArrayRegion,
        GetIntArrayRegion,
        GetLongArrayRegion,
        GetFloatArrayRegion,
        GetDoubleArrayRegion,

        SetBooleanArrayRegion,
        SetByteArrayRegion,
        SetCharArrayRegion,
        SetShortArrayRegion,
        SetIntArrayRegion,
        SetLongArrayRegion,
        SetFloatArrayRegion,
        SetDoubleArrayRegion,

        RegisterNatives,
        UnregisterNatives,

        MonitorEnter,
        MonitorExit,

        GetJavaVM,

        GetStringRegion,
        GetStringUTFRegion,

        GetPrimitiveArrayCritical,
        ReleasePrimitiveArrayCritical,

        GetStringCritical,
        ReleaseStringCritical,

        NewWeakGlobalRef,
        DeleteWeakGlobalRef,

        ExceptionCheck,

        NewDirectByteBuffer,
        GetDirectBufferAddress,
        GetDirectBufferCapacity,

        /* New JNI 1.6 Features */

        GetObjectRefType,

        /* Module Features */

        GetModule,
};
