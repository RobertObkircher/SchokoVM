#include <iostream>
#include <vector>
#include <dlfcn.h>

#define _JNI_IMPLEMENTATION_

#include "jni.h"
#include "classloading.hpp"
#include "memory.hpp"
#include "parser.hpp"

#define UNIMPLEMENTED(x) std::cerr << x; exit(42);

#define LOG(x)

/**
 * The Invocation API
 *
 * https://docs.oracle.com/en/java/javase/11/docs/specs/jni/invocation.html
 */

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_GetDefaultJavaVMInitArgs(void *args) {
    LOG("JNI_GetDefaultJavaVMInitArgs");
    auto *vm_args = static_cast<JavaVMInitArgs *>(args);
    if (vm_args->version != JNI_VERSION_10) {
        return JNI_EVERSION;
    }

    vm_args->ignoreUnrecognized = false;
    vm_args->nOptions = 0;
    vm_args->options = nullptr;

    return JNI_OK;
}

extern const struct JNIInvokeInterface_ jni_invoke_interface;
extern const struct JNINativeInterface_ jni_native_interface;

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_CreateJavaVM(JavaVM **pvm, void **penv, void *args) {
    auto *vm_args = static_cast<JavaVMInitArgs *>(args);

    std::string bootclasspath_option{"-Xbootclasspath:"};
    std::string classpath_option{"-Djava.class.path="};

    std::string bootclasspath{};
    std::string classpath{};

    for (int i = 0; i < vm_args->nOptions; ++i) {
        std::string option{vm_args->options[i].optionString};
        if (option.starts_with(bootclasspath_option)) {
            bootclasspath = option.substr(bootclasspath_option.size());
        } else if (option.starts_with(classpath_option)) {
            classpath = option.substr(classpath_option.size());
        }
    }

    auto dlsym_handle = dlopen("/usr/lib/jvm/java-11-openjdk/lib/libverify.so", RTLD_LAZY | RTLD_GLOBAL);
    if (auto message = dlerror(); message != nullptr) {
        std::cerr << "dlopen failed: " << message << "\n";
        abort();
    }
    dlsym_handle = dlopen("/usr/lib/jvm/java-11-openjdk/lib/libjava.so", RTLD_LAZY | RTLD_GLOBAL);
    if (auto message = dlerror(); message != nullptr) {
        std::cerr << "dlopen failed: " << message << "\n";
        abort();
    }


    // TODO remove classpath
    BootstrapClassLoader::get().initialize_with_boot_classpath(bootclasspath + ":" + classpath);

    auto *thread = new Thread();
    thread->stack.memory.resize(1024 * 1024 / sizeof(Value)); // 1mb for now

    // TODO should be something like thread.get_jni()
    auto *native = new JNINativeInterface_{jni_native_interface};
    native->reserved0 = thread;

    *pvm = new JavaVM{&jni_invoke_interface};
    *penv = thread->jni_env = new JNIEnv{native}; // TODO store JNIEnv as member of thread instead of allocating?
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
    LOG("DestroyJavaVM");
    delete vm;
    return JNI_OK;
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
    LOG("ExceptionOccurred");
    return nullptr;
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

/**
 *    CallReturntypeMethod
 *    CallNonvirtualReturntypeMethod
 *    CallStaticReturntypeMethod
 */

#define CALL_HELPER(Name, ArgType, NextArg)                                                                            \
static jint Name(JNIEnv *env, jclass java_class, bool is_virtual,                                                      \
                 jobject java_object, jmethodID methodID, ArgType args, Value &result) {                               \
    auto *thread = static_cast<Thread *>(env->functions->reserved0);                                                   \
    auto *object = (Object *) java_object;                                                                             \
    auto *clazz = (ClassFile *) java_class;                                                                            \
    auto *method = (method_info *) methodID;                                                                           \
                                                                                                                       \
    size_t saved_operand_stack_top = thread->stack.memory_used;                                                        \
    thread->stack.memory_used += method->parameter_count;                                                              \
    if (thread->stack.memory_used > thread->stack.memory.size()) {                                                     \
        return JNI_ENOMEM;                                                                                             \
    }                                                                                                                  \
                                                                                                                       \
    if (is_virtual) {                                                                                                  \
        auto *declared_clazz = clazz;                                                                                  \
        auto *declared_method = method;                                                                                \
                                                                                                                       \
        if (method_selection(object->clazz, declared_clazz, declared_method, clazz, method)) {                         \
            return JNI_ERR;                                                                                            \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    size_t offset = saved_operand_stack_top;                                                                           \
    assert(method->is_static() == (object == nullptr));                                                                \
    if (!method->is_static()) {                                                                                        \
        thread->stack.memory[offset] = Value(Reference{object});                                                       \
        ++offset;                                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    MethodDescriptorParts parts{method->descriptor_index->value.c_str()};                                              \
    for (; !parts->is_return; ++parts) {                                                                               \
        thread->stack.memory[offset] = NextArg;                                                                        \
        offset += parts->category;                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    result = interpret(*thread, clazz, method);                                                                        \
    thread->stack.memory_used = saved_operand_stack_top;                                                               \
    return JNI_OK;                                                                                                     \
}                                                                                                                      \

CALL_HELPER(call, const jvalue *, Value((s8) args->j); ++args)
CALL_HELPER(call_v, va_list, (Value((s8) va_arg(args, jvalue).j)))
#undef CALL_HELPER

// static jint call(JNIEnv *env, jclass java_class, bool is_virtual, jobject java_object, jmethodID methodID, const jvalue *args, Value &result);
// jobject CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...);
// jobject CallNonvirtualObjectMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...);
// jobject CallStaticObjectMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...);
#define CALL(ReturnType, Name, Return)                                                                                 \
ReturnType Call##Name##Method(JNIEnv *env, jobject object, jmethodID methodID, ...) {                                  \
    LOG("Call" #Name "Method");                                                                                        \
    Value result;                                                                                                      \
    va_list args;                                                                                                      \
    va_start(args, methodID);                                                                                          \
    call_v(env, nullptr, true, object, methodID, args, result);                                                        \
    va_end(args);                                                                                                      \
    Return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType Call##Name##MethodV(JNIEnv *env, jobject object, jmethodID methodID, va_list args) {                        \
    LOG("Call" #Name "MethodV");                                                                                       \
    Value result;                                                                                                      \
    call_v(env, nullptr, true, object, methodID, args, result);                                                        \
    return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType Call##Name##MethodA(JNIEnv *env, jobject object, jmethodID methodID, const jvalue *args) {                  \
    LOG("Call" #Name "MethodA");                                                                                       \
    Value result;                                                                                                      \
    call(env, nullptr, true, object, methodID, args, result);                                                          \
    return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType CallNonvirtual##Name##Method(JNIEnv *env, jobject object, jclass clazz, jmethodID methodID, ...) {          \
    LOG("CallNonvirtual" #Name "Method");                                                                              \
    Value result;                                                                                                      \
    va_list args;                                                                                                      \
    va_start(args, methodID);                                                                                          \
    call_v(env, clazz, false, object, methodID, args, result);                                                         \
    va_end(args);                                                                                                      \
    Return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType CallNonvirtual##Name##MethodV(JNIEnv *env, jobject object, jclass clazz, jmethodID methodID, va_list args) {\
    LOG("CallNonvirtual" #Name "MethodV");                                                                             \
    Value result;                                                                                                      \
    call_v(env, clazz, false, object, methodID, args, result);                                                         \
    return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType CallNonvirtual##Name##MethodA(JNIEnv *env, jobject object, jclass clazz, jmethodID methodID, const jvalue *args) {\
    LOG("CallNonvirtual" #Name "MethodA");                                                                             \
    Value result;                                                                                                      \
    call(env, clazz, false, object, methodID, args, result);                                                           \
    return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType CallStatic##Name##Method(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {                              \
    LOG("CallStatic" #Name "Method");                                                                                  \
    Value result;                                                                                                      \
    va_list args;                                                                                                      \
    va_start(args, methodID);                                                                                          \
    call_v(env, clazz, false, nullptr, methodID, args, result);                                                        \
    va_end(args);                                                                                                      \
    Return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType CallStatic##Name##MethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {                    \
    LOG("CallStatic" #Name "MethodV");                                                                                 \
    Value result;                                                                                                      \
    call_v(env, clazz, false, nullptr, methodID, args, result);                                                        \
    return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \
ReturnType CallStatic##Name##MethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {              \
    LOG("CallStatic" #Name "MethodA");                                                                                 \
    Value result;                                                                                                      \
    call(env, clazz, false, nullptr, methodID, args, result);                                                          \
    return (ReturnType) result.s8;                                                                                     \
}                                                                                                                      \

CALL(jobject, Object, return);
CALL(jboolean, Boolean, return);
CALL(jbyte, Byte, return);
CALL(jchar, Char, return);
CALL(jshort, Short, return);
CALL(jint, Int, return);
CALL(jlong, Long, return);
CALL(jfloat, Float, return);
CALL(jdouble, Double, return);
CALL(void, Void,);

#undef CALL

jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    LOG("GetFieldID");
    auto *thread = (Thread *) env->functions->reserved0;
    return (jfieldID) find_field((ClassFile*) clazz, name, sig, thread->current_exception);
}

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    LOG("GetStaticFieldID");
    auto *thread = (Thread *) env->functions->reserved0;
    return (jfieldID) find_field((ClassFile*) clazz, name, sig, thread->current_exception);
}


/*
 * GetTypeField
 * SetTypeField
 * GetStaticTypeField
 * SetStaticTypeField
 */

#define FIELD(JavaType, Name, Variant, CppType)                                                                        \
JavaType Get##Name##Field(JNIEnv *, jobject obj, jfieldID fieldID) {                                                   \
    LOG("Get" #Name "Field");                                                                                          \
    size_t index = ((field_info *) fieldID)->index;                                                                    \
    return (JavaType) Reference{obj}.data<Value>()[index].Variant;                                                     \
}                                                                                                                      \
void Set##Name##Field(JNIEnv *, jobject obj, jfieldID fieldID, JavaType val) {                                         \
    LOG("Set" #Name "Field");                                                                                          \
    size_t index = ((field_info *) fieldID)->index;                                                                    \
    Reference{obj}.data<Value>()[index].Variant = (CppType) val;                                                       \
}                                                                                                                      \
JavaType GetStatic##Name##Field(JNIEnv *, jclass clazz, jfieldID fieldID) {                                            \
    LOG("GetStatic" #Name "Field");                                                                                    \
    size_t index = ((field_info *) fieldID)->index;                                                                    \
    return (JavaType) ((ClassFile *) clazz)->static_field_values[index].Variant;                                       \
}                                                                                                                      \
void SetStatic##Name##Field(JNIEnv *, jclass clazz, jfieldID fieldID, JavaType val) {                                  \
    LOG("SetStatic" #Name "Field");                                                                                    \
    size_t index = ((field_info *) fieldID)->index;                                                                    \
    ((ClassFile *) clazz)->static_field_values[index].Variant = (CppType) val;                                         \
}                                                                                                                      \

FIELD(jobject, Object, reference.memory, void *)
FIELD(jboolean, Boolean, s4, u1);
FIELD(jbyte, Byte, s4, u1);
FIELD(jchar, Char, s4, u2);
FIELD(jshort, Short, s4, u2);
FIELD(jint, Int, s4, s4);
FIELD(jlong, Long, s8, s8);
FIELD(jfloat, Float, float_, float);
FIELD(jdouble, Double, double_, double);
#undef FIELD

jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    LOG("GetStaticMethodID");

    auto *clazzz = (ClassFile *) (clazz);

    auto method_iter = std::find_if(clazzz->methods.begin(), clazzz->methods.end(),
                                    [name, sig](const method_info &m) {
                                        return m.name_index->value == name && m.descriptor_index->value == sig;
                                    }
    );

    if (method_iter == clazzz->methods.end()) {
        return nullptr;
    }

    assert(method_iter->is_static());
    method_info *m = &*method_iter;
    return (jmethodID) m;
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
    LOG("NewByteArray");

    auto *clazz = BootstrapClassLoader::get().load("[B");
    return (jbyteArray) Heap::get().new_array<u1>(clazz, len).memory;
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
    LOG("SetByteArrayRegion");

    auto *x = (Object *) (array);
    Reference r{x};
    u1 *data = r.data<u1>();

    memcpy(data + start, buf, static_cast<size_t>(len));
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
