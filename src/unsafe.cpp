#include "unsafe.hpp"

#include <string>

#include "classfile.hpp"
#include "memory.hpp"
#include "classloading.hpp"
#include "exceptions.hpp"

#define UNIMPLEMENTED(x) std::cerr << x; exit(42);
#define LOG(x)

JNICALL static jobject Unsafe_GetObjectVolatile(JNIEnv *env, jobject unsafe, jobject obj, jlong offset) {
    LOG("Unsafe_GetObjectVolatile");
    // TODO "get with volatile load semantics, otherwise identical to getObject()"
    auto field = reinterpret_cast<Value *>(reinterpret_cast<char *>(obj) + offset);
    return reinterpret_cast<jobject>(field->reference.memory);
}

JNICALL static jint Unsafe_GetIntVolatile(JNIEnv *env, jobject unsafe, jobject obj, jlong offset) {
    LOG("Unsafe_GetIntVolatile");
    // TODO "volatile"
    auto field = reinterpret_cast<Value *>(reinterpret_cast<char *>(obj) + offset);
    return field->s4;
}

JNICALL static jlong Unsafe_ObjectFieldOffset1(JNIEnv *env, jobject unsafe, jclass cls, jstring name) {
    LOG("Unsafe_ObjectFieldOffset1");
    auto clazz = reinterpret_cast<ClassFile *>(cls);

    auto data = env->GetStringUTFChars(name, nullptr);
    auto str = std::string_view(data);

    for (const auto &f : clazz->fields) {
        if (f.name_index->value == str) {
            env->ReleaseStringUTFChars(name, data);
            return static_cast<jlong>(offset_of_array_after_header<Object, Value>() + sizeof(Value) * f.index);
        }
    }

    env->ReleaseStringUTFChars(name, data);
    throw std::runtime_error("TODO: not found?");
}

JNICALL static void Unsafe_EnsureClassInitialized0(JNIEnv *env, jobject unsafe, jclass cls) {
    LOG("Unsafe_EnsureClassInitialized0");
    auto clazz = reinterpret_cast<ClassFile *>(cls);
    auto *thread = static_cast<Thread *>(env->functions->reserved0);

    if (resolve_class(clazz) == Exception) {
        return;
    }
    if (initialize_class(clazz, *thread) == Exception) {
        return;
    }
}

JNICALL static jint Unsafe_ArrayBaseOffset0(JNIEnv *env, jobject unsafe, jclass cls) {
    LOG("Unsafe_ArrayBaseOffset0");
    auto clazz = reinterpret_cast<ClassFile *>(cls);

    if (!clazz->is_array()) {
        auto *thread = static_cast<Thread *>(env->functions->reserved0);
        throw_new(*thread, "java/lang/InvalidClassException");
        return 0;
    } else {
        return static_cast<jint>(clazz->offset_of_array_after_header);
    }
}

JNICALL static jint Unsafe_ArrayIndexScale0(JNIEnv *env, jobject unsafe, jclass cls) {
    LOG("Unsafe_ArrayIndexScale0");
    auto clazz = reinterpret_cast<ClassFile *>(cls);

    if (!clazz->is_array()) {
        // TODO java_lang_InvalidClassException
        throw std::runtime_error("TODO: java_lang_InvalidClassException");
    } else {
        return static_cast<jint>(clazz->element_size);
    }
}

template<typename T>
static bool compare_and_set(jobject obj, jlong offset, T expected, T desired) {
    auto *field = (reinterpret_cast<T *>(reinterpret_cast<char *>(obj) + offset));
    return __atomic_compare_exchange_n(field, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

JNICALL static jboolean
Unsafe_CompareAndSetObject(JNIEnv *env, jobject unsafe, jobject obj, jlong offset, jobject expected, jobject desired) {
    LOG("Unsafe_CompareAndSetObject");
    // TODO "volatile semantics"
    return compare_and_set(obj, offset, expected, desired);
}

JNICALL static jboolean
Unsafe_CompareAndSetInt(JNIEnv *env, jobject unsafe, jobject obj, jlong offset, jint expected, jint desired) {
    LOG("Unsafe_CompareAndSetInt");
    return compare_and_set(obj, offset, expected, desired);
}

JNICALL static jboolean
Unsafe_CompareAndSetLong(JNIEnv *env, jobject unsafe, jobject obj, jlong offset, jlong expected, jlong desired) {
    LOG("Unsafe_CompareAndSetLong");
    return compare_and_set(obj, offset, expected, desired);
}


JNICALL static jint Unsafe_AddressSize0(JNIEnv *env, jobject unsafe) {
    return sizeof(void *);
}

JNICALL void static Unsafe_LoadFence(JNIEnv *env, jobject unsafe) {
    LOG("Unsafe_LoadFence");
    std::atomic_thread_fence(std::memory_order_acquire);
}

JNICALL void static Unsafe_StoreFence(JNIEnv *env, jobject unsafe) {
    LOG("Unsafe_StoreFence");
    std::atomic_thread_fence(std::memory_order_release);
}

JNICALL void static Unsafe_FullFence(JNIEnv *env, jobject unsafe) {
    LOG("Unsafe_FullFence");
    std::atomic_thread_fence(std::memory_order_acq_rel);
}

JNICALL static jboolean Unsafe_isBigEndian0(JNIEnv *env, jobject unsafe) {
    return std::endian::native == std::endian::big;
}

JNICALL static jboolean Unsafe_unalignedAccess0(JNIEnv *env, jobject unsafe) {
    // TODO ???
    return false;
}

// --- Register

#define ADR "J"
#define LANG "Ljava/lang/"
#define OBJ LANG "Object;"
#define CLS LANG "Class;"
#define FLD LANG "reflect/Field;"
#define THR LANG "Throwable;"

#define DC_Args  LANG "String;[BII" LANG "ClassLoader;" "Ljava/security/ProtectionDomain;"
#define DAC_Args CLS "[B[" OBJ

#define CC(str) (const_cast<char *>(str))  /*cast a literal from (const char*)*/

#define DECLARE_GETPUTOOP(Type, Desc) \
    {CC("get" #Type),      CC("(" OBJ "J)" #Desc),       Unsafe_Get##Type}, \
    {CC("put" #Type),      CC("(" OBJ "J" #Desc ")V"),   Unsafe_Put##Type}, \
    {CC("get" #Type "Volatile"),      CC("(" OBJ "J)" #Desc),       Unsafe_Get##Type##Volatile}, \
    {CC("put" #Type "Volatile"),      CC("(" OBJ "J" #Desc ")V"),   Unsafe_Put##Type##Volatile}

static JNINativeMethod methods[] = {
//        {CC("getObject"), CC("(" OBJ "J)" OBJ ""), Unsafe_GetObject},
//        {CC("putObject"), CC("(" OBJ "J" OBJ ")V"), Unsafe_PutObject},
        {CC("getObjectVolatile"),       CC("(" OBJ "J)" OBJ ""),          reinterpret_cast<void *>(Unsafe_GetObjectVolatile)},
//        {CC("putObjectVolatile"), CC("(" OBJ "J" OBJ ")V"), Unsafe_PutObjectVolatile},
        {CC("getIntVolatile"),          CC("(" OBJ "J)I"),                reinterpret_cast<void *>(Unsafe_GetIntVolatile)},

//        {CC("getUncompressedObject"), CC("(" ADR ")" OBJ), Unsafe_GetUncompressedObject},
//
//        DECLARE_GETPUTOOP(Boolean, Z),
//        DECLARE_GETPUTOOP(Byte, B),
//        DECLARE_GETPUTOOP(Short, S),
//        DECLARE_GETPUTOOP(Char, C),
//        DECLARE_GETPUTOOP(Int, I),
//        DECLARE_GETPUTOOP(Long, J),
//        DECLARE_GETPUTOOP(Float, F),
//        DECLARE_GETPUTOOP(Double, D),
//
//        {CC("allocateMemory0"), CC ("(J)" ADR), Unsafe_AllocateMemory0},
//        {CC("reallocateMemory0"), CC ("(" ADR "J)" ADR), Unsafe_ReallocateMemory0},
//        {CC("freeMemory0"), CC ("(" ADR ")V"), Unsafe_FreeMemory0},
//
//        {CC("objectFieldOffset0"), CC ("(" FLD ")J"), Unsafe_ObjectFieldOffset0},
        {CC("objectFieldOffset1"),      CC ("(" CLS LANG "String;)J"),    reinterpret_cast<void *>(Unsafe_ObjectFieldOffset1)},
//        {CC("staticFieldOffset0"), CC ("(" FLD ")J"), Unsafe_StaticFieldOffset0},
//        {CC("staticFieldBase0"), CC ("(" FLD ")" OBJ), Unsafe_StaticFieldBase0},
        {CC("ensureClassInitialized0"), CC ("(" CLS ")V"),                reinterpret_cast<void *>(Unsafe_EnsureClassInitialized0)},
        {CC("arrayBaseOffset0"),        CC ("(" CLS ")I"),                reinterpret_cast<void *>(Unsafe_ArrayBaseOffset0)},
        {CC("arrayIndexScale0"),        CC ("(" CLS ")I"),                reinterpret_cast<void *>(Unsafe_ArrayIndexScale0)},
        {CC("addressSize0"),            CC ("()I"),                       reinterpret_cast<void *>(Unsafe_AddressSize0)},
//        {CC("pageSize"), CC ("()I"), Unsafe_PageSize},
//
//        {CC("defineClass0"), CC ("(" DC_Args ")" CLS), Unsafe_DefineClass0},
//        {CC("allocateInstance"), CC ("(" CLS ")" OBJ), Unsafe_AllocateInstance},
//        {CC("throwException"), CC ("(" THR ")V"), Unsafe_ThrowException},
        {CC("compareAndSetObject"),     CC ("(" OBJ "J" OBJ "" OBJ ")Z"), reinterpret_cast<void *>(Unsafe_CompareAndSetObject)},
        {CC("compareAndSetInt"),        CC ("(" OBJ "J""I""I"")Z"),       reinterpret_cast<void *>(Unsafe_CompareAndSetInt)},
        {CC("compareAndSetLong"),       CC ("(" OBJ "J""J""J"")Z"),       reinterpret_cast<void *>(Unsafe_CompareAndSetLong)},
//        {CC("compareAndExchangeObject"), CC ("(" OBJ "J" OBJ "" OBJ ")" OBJ), Unsafe_CompareAndExchangeObject},
//        {CC("compareAndExchangeInt"), CC ("(" OBJ "J""I""I"")I"), Unsafe_CompareAndExchangeInt},
//        {CC("compareAndExchangeLong"), CC ("(" OBJ "J""J""J"")J"), Unsafe_CompareAndExchangeLong},
//
//        {CC("park"), CC("(ZJ)V"), Unsafe_Park},
//        {CC("unpark"), CC ("(" OBJ ")V"), Unsafe_Unpark},
//
//        {CC("getLoadAverage0"), CC ("([DI)I"), Unsafe_GetLoadAverage0},
//
//        {CC("copyMemory0"), CC("(" OBJ "J" OBJ "JJ)V"), Unsafe_CopyMemory0},
//        {CC("copySwapMemory0"), CC("(" OBJ "J" OBJ "JJJ)V"), Unsafe_CopySwapMemory0},
//        {CC("setMemory0"), CC("(" OBJ "JJB)V"), Unsafe_SetMemory0},
//
//        {CC("defineAnonymousClass0"), CC("(" DAC_Args ")" CLS), Unsafe_DefineAnonymousClass0},
//
//        {CC("shouldBeInitialized0"), CC("(" CLS ")Z"), Unsafe_ShouldBeInitialized0},
//
        {CC("loadFence"),               CC("()V"),                        reinterpret_cast<void *>(Unsafe_LoadFence)},
        {CC("storeFence"),              CC("()V"),                        reinterpret_cast<void *>(Unsafe_StoreFence)},
        {CC("fullFence"),               CC("()V"),                        reinterpret_cast<void *>(Unsafe_FullFence)},

        {CC("isBigEndian0"),            CC("()Z"),                        reinterpret_cast<void *>(Unsafe_isBigEndian0)},
        {CC("unalignedAccess0"),        CC("()Z"),                        reinterpret_cast<void *>(Unsafe_unalignedAccess0)}
};

#undef OBJ
#undef CLS
#undef STR
#undef FLD
#undef MHD
#undef CTR
#undef PD

extern "C" {
_JNI_IMPORT_OR_EXPORT_
void JNICALL Java_jdk_internal_misc_Unsafe_registerNatives(JNIEnv *env, jclass clazz) {
    env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(JNINativeMethod));
}
}
