#include <iostream>
#include <vector>
#include <dlfcn.h>

#define _JNI_IMPLEMENTATION_

#include <locale>
#include <codecvt>

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

std::string java_home{};

_JNI_IMPORT_OR_EXPORT_ jint JNICALL
JNI_CreateJavaVM(JavaVM **pvm, void **penv, void *args) {
    auto *vm_args = static_cast<JavaVMInitArgs *>(args);

    static const std::string BOOTCLASSPATH_OPTION = "-Xbootclasspath:";
    static const std::string CLASSPATH_option{"-Djava.class.path="};
    static const std::string JAVAHOME_OPTION{"-Xjavahome:"};

    std::string bootclasspath{};
    std::string classpath{};

    for (int i = 0; i < vm_args->nOptions; ++i) {
        std::string option{vm_args->options[i].optionString};
        if (option.starts_with(BOOTCLASSPATH_OPTION)) {
            bootclasspath = option.substr(BOOTCLASSPATH_OPTION.size());
        } else if (option.starts_with(CLASSPATH_option)) {
            classpath = option.substr(CLASSPATH_option.size());
        } else if (option.starts_with(JAVAHOME_OPTION)) {
            java_home = option.substr(JAVAHOME_OPTION.size());
        }
    }

    std::string libverify_path = java_home + "/lib/libverify" + LIB_EXTENSION;
    dlopen(libverify_path.c_str(),
           RTLD_LAZY | RTLD_GLOBAL);
    if (auto message = dlerror(); message != nullptr) {
        std::cerr << "dlopen failed: " << message << "\n";
        abort();
    }
    std::string libjava_path = java_home + "/lib/libjava" + LIB_EXTENSION;
    dlopen(libjava_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
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

    BootstrapClassLoader::get().resolve_and_initialize_constants(*thread);

    {
        auto class_ThreadGroup = reinterpret_cast<jclass>(BootstrapClassLoader::constants().java_lang_ThreadGroup);
        auto class_Thread = reinterpret_cast<jclass>(BootstrapClassLoader::constants().java_lang_Thread);

        jmethodID thread_group_init = thread->jni_env->GetMethodID(class_ThreadGroup, "<init>", "()V");
        jmethodID thread_group_sub_init = thread->jni_env->GetMethodID(class_ThreadGroup, "<init>",
                                                                       "(Ljava/lang/ThreadGroup;Ljava/lang/String;)V");
        assert(thread_group_init);
        assert(thread_group_sub_init);

        auto thread_group_system_obj = thread->jni_env->AllocObject(class_ThreadGroup);
        // TODO invokeSpecial
        thread->jni_env->CallNonvirtualVoidMethod(thread_group_system_obj, class_ThreadGroup, thread_group_init);
        if (thread->jni_env->ExceptionCheck()) {
            abort();
        }

        auto thread_group_main_obj = thread->jni_env->AllocObject(class_ThreadGroup);
        std::u16string main_str{u"main"};
        auto main_str_obj = thread->jni_env->NewString(reinterpret_cast<const jchar *>(main_str.c_str()),
                                                       static_cast<jsize>(main_str.length()));
        thread->jni_env->CallNonvirtualVoidMethod(thread_group_main_obj, class_ThreadGroup, thread_group_sub_init,
                                                  thread_group_system_obj, main_str_obj);
        if (thread->jni_env->ExceptionCheck()) {
            abort();
        }

        auto thread_obj = thread->jni_env->AllocObject(class_Thread);
        auto thread_ref = Reference{thread_obj};
        // Important to set this before calling the constructor, because the Thread calls currentThread()
        thread->thread_object = Reference{thread_obj};
        [[maybe_unused]] auto thread_priority_field = reinterpret_cast<ClassFile *>(class_Thread)->fields[1];
        assert(thread_priority_field.name_index->value == "priority" && thread_priority_field.index == 1);
        thread_ref.data<Value>()[1] /* thread.priority */ = Value{5 /* Thread.NORM_PRIORITY */ };

        jmethodID thread_init = thread->jni_env->GetMethodID(class_Thread, "<init>",
                                                             "(Ljava/lang/ThreadGroup;Ljava/lang/String;)V");
        assert(thread_init);
        thread->jni_env->CallNonvirtualVoidMethod(thread_obj, class_Thread, thread_init, thread_group_main_obj,
                                                  main_str_obj);
        if (thread->jni_env->ExceptionCheck()) {
            abort();
        }
    }

    auto *system = BootstrapClassLoader::get().load_or_throw("java/lang/System");
    assert(system);
    jmethodID method = thread->jni_env->GetStaticMethodID((jclass) system, "initPhase1", "()V");
    assert(method);
    thread->jni_env->CallStaticVoidMethod((jclass) system, method);
    if (thread->jni_env->ExceptionCheck()) {
        abort();
    }

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
        (JNIEnv *env, const char *utf8_mod) {
    LOG("FindClass");

    // TODO should be the class loader of the class that declared the native method
    auto clazz = BootstrapClassLoader::get().load(utf8_mod);

    // TODO initialize_class?
    return reinterpret_cast<jclass>(clazz);
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

jthrowable ExceptionOccurred(JNIEnv *env) {
    LOG("ExceptionOccurred");
    auto *thread = static_cast<Thread *>(env->functions->reserved0);
    return (jthrowable) thread->current_exception.memory;
}

void ExceptionDescribe(JNIEnv *env) {
    UNIMPLEMENTED("ExceptionDescribe");
}

void ExceptionClear(JNIEnv *env) {
    LOG("ExceptionClear");
    auto *thread = static_cast<Thread *>(env->functions->reserved0);
    thread->current_exception = JAVA_NULL;
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
    // TODO GC
    return lobj;
}

void DeleteGlobalRef
        (JNIEnv *env, jobject gref) {
    UNIMPLEMENTED("DeleteGlobalRef");
}

void DeleteLocalRef
        (JNIEnv *env, jobject obj) {
    LOG("DeleteLocalRef");
    // TODO implemence once we have GC
}

jboolean IsSameObject
        (JNIEnv *env, jobject obj1, jobject obj2) {
    UNIMPLEMENTED("IsSameObject");
}

jobject NewLocalRef
        (JNIEnv *env, jobject ref) {
    UNIMPLEMENTED("NewLocalRef");
}

// TODO should this be configureable?
//  jdk11u-dev/src/hotspot/share/runtime/globals.hpp
//  https://docs.oracle.com/javase/9/docs/specs/jni/functions.html#ensurelocalcapacity
static const jint MaxJNILocalCapacity = -1;

jint EnsureLocalCapacity
        (JNIEnv *env, jint capacity) {
    LOG("EnsureLocalCapacity");

    jint ret;
    if (capacity >= 0 &&
        ((MaxJNILocalCapacity <= 0) || (capacity <= MaxJNILocalCapacity))) {
        ret = JNI_OK;
    } else {
        ret = JNI_ERR;
    }

    return ret;
}

jobject AllocObject
        (JNIEnv *env, jclass cls) {
    LOG("AllocObject");
    auto clazz = reinterpret_cast<ClassFile *>(cls);
    return reinterpret_cast<jobject>(Heap::get().new_instance(clazz).memory);
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
    LOG("GetObjectClass");
    auto ref = Reference{obj};
    ClassFile *clazz = ref.object()->clazz;
    return reinterpret_cast<jclass>(clazz);
}

jboolean IsInstanceOf
        (JNIEnv *env, jobject obj, jclass clazz) {
    UNIMPLEMENTED("IsInstanceOf");
}

jmethodID GetMethodID
        (JNIEnv *env, jclass cls, const char *name, const char *sig) {
    LOG("GetMethodID");
    auto *thread = static_cast<Thread *>(env->functions->reserved0);
    auto *clazz = (ClassFile *) cls;

    if (resolve_class(clazz) == Exception)
        return nullptr;

    if (initialize_class(clazz, *thread) == Exception)
        return nullptr;

    auto result = method_resolution(clazz, name, sig);
    return reinterpret_cast<jmethodID>(result);
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
    thread->stack.memory_used += method->stack_slots_for_parameters;                                                              \
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
    return (jfieldID) find_field((ClassFile *) clazz, name, sig, thread->current_exception);
}

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    LOG("GetStaticFieldID");
    auto *thread = (Thread *) env->functions->reserved0;
    return (jfieldID) find_field((ClassFile *) clazz, name, sig, thread->current_exception);
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

    // TODO "GetStaticMethodID() causes an uninitialized class to be initialized."

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
        (JNIEnv *env, const jchar *utf16_data, jsize len) {
    LOG("NewString");
    std::u16string_view str{reinterpret_cast<const char16_t *>(utf16_data), static_cast<size_t>(len)};
    auto ref = Heap::get().make_string(str);
    return reinterpret_cast<jstring>(ref.memory);
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
        (JNIEnv *env, const char *utf8_mod) {
    LOG("NewStringUTF");
    auto str = Heap::get().make_string(utf8_mod);
    return reinterpret_cast<jstring>(str.memory);
}

jsize GetStringUTFLength
        (JNIEnv *env, jstring str) {
    UNIMPLEMENTED("GetStringUTFLength");
}

const char *GetStringUTFChars
        (JNIEnv *env, jstring str, jboolean *isCopy) {
    LOG("GetStringUTFChars");
    // TODO this should really be using modified utf8 and not real utf8
    auto ref = Reference{str};
    auto charArray = ref.data<Value>()[0].reference;
    auto utf16_length_bytes = static_cast<size_t>(charArray.object()->length);
    auto *utf16_data = charArray.data<u1>();

    std::string utf8_string = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(
            (char16_t *) utf16_data, (char16_t *) (utf16_data + utf16_length_bytes));
    auto utf8_length = utf8_string.size();

    char *result = new char[utf8_length + 1];
    std::strcpy(result, utf8_string.c_str());

    if (isCopy != nullptr) {
        *isCopy = JNI_TRUE;
    }
    return result;
}

void ReleaseStringUTFChars
        (JNIEnv *env, jstring str, const char *chars) {
    LOG("ReleaseStringUTFChars");
    delete[] chars;
}

jsize GetArrayLength
        (JNIEnv *env, jarray array) {
    LOG("GetArrayLength");
    auto ref = Reference{array};
    return ref.object()->length;
}

jobjectArray NewObjectArray
        (JNIEnv *env, jsize len, jclass clazz, jobject init) {
    LOG("NewObjectArray");
    return reinterpret_cast<jobjectArray>(
            Heap::get().new_array<Reference>(reinterpret_cast<ClassFile *>(clazz), len).memory
    );
}

jobject GetObjectArrayElement
        (JNIEnv *env, jobjectArray array, jsize index) {
    UNIMPLEMENTED("GetObjectArrayElement");
}

void SetObjectArrayElement
        (JNIEnv *env, jobjectArray array, jsize index, jobject val) {
    LOG("SetObjectArrayElement");
    auto ref = Reference{array};
    ref.data<Reference>()[index] = Reference{val};
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
    LOG("GetByteArrayRegion");
    auto ref = Reference{array};
    memcpy(buf, ref.data<s1>() + start, static_cast<size_t>(len) * sizeof(u1));
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
    auto *java_class = (ClassFile *) clazz;
    for (int i = 0; i < nMethods; i++) {
        const auto &name = methods[i].name;
        const auto &sig = methods[i].signature;
        const auto &ptr = methods[i].fnPtr;
        auto method_iter = std::find_if(java_class->methods.begin(), java_class->methods.end(),
                                        [name, sig](const method_info &m) {
                                            return m.name_index->value == name && m.descriptor_index->value == sig;
                                        }
        );

        if (method_iter == java_class->methods.end()) {
            // TODO error
            std::cerr << "not found" << "\n";
            return JNI_ERR;
        }

        method_iter->native_function = NativeFunction{&*method_iter, ptr};
    }
    return JNI_OK;
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
    LOG("ExceptionCheck");
    auto *thread = (Thread *) env->functions->reserved0;
    return thread->current_exception != JAVA_NULL;
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
