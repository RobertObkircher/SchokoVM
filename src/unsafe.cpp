#include "unsafe.hpp"

#include <string>

#include "classfile.hpp"
#include "memory.hpp"
#include "classloading.hpp"

#define UNIMPLEMENTED(x) std::cerr << x; exit(42);
#define LOG(x)

JNIEXPORT jobject JNICALL
static Unsafe_GetObjectVolatile(JNIEnv *env, jobject unsafe, jobject obj, jlong offset) {
    // TODO "get with volatile load semantics, otherwise identical to getObject()"
    auto field = reinterpret_cast<Value *>(reinterpret_cast<char *>(obj) + offset);
    return reinterpret_cast<jobject>(field->reference.memory);
}

JNIEXPORT jlong JNICALL
static Unsafe_ObjectFieldOffset1(JNIEnv *env, jobject unsafe, jclass cls, jstring name) {
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


JNIEXPORT jint JNICALL
static Unsafe_ArrayBaseOffset0(JNIEnv *env, jobject unsafe, jclass cls) {
    LOG("JVM_GetClassModifiers");
    auto clazz = reinterpret_cast<ClassFile *>(cls);

    if (!clazz->is_array()) {
        // TODO java_lang_InvalidClassException
        throw std::runtime_error("TODO: java_lang_InvalidClassException");
    } else if (clazz->array_element_type->name() == "boolean" || clazz->array_element_type->name() == "byte") {
        return offset_of_array_after_header<Object, s1>();
    } else if (clazz->array_element_type->name() == "char") {
        return offset_of_array_after_header<Object, u2>();
    } else if (clazz->array_element_type->name() == "short") {
        return offset_of_array_after_header<Object, s2>();
    } else if (clazz->array_element_type->name() == "int") {
        return offset_of_array_after_header<Object, s4>();
    } else if (clazz->array_element_type->name() == "long") {
        return offset_of_array_after_header<Object, s8>();
    } else if (clazz->array_element_type->name() == "float") {
        return offset_of_array_after_header<Object, float>();
    } else if (clazz->array_element_type->name() == "double") {
        return offset_of_array_after_header<Object, double>();
    } else {
        // object
        return offset_of_array_after_header<Object, Reference>();
    }
}

JNIEXPORT jint JNICALL
static Unsafe_ArrayIndexScale0(JNIEnv *env, jobject unsafe, jclass cls) {
    LOG("Unsafe_ArrayIndexScale0");
    auto clazz = reinterpret_cast<ClassFile *>(cls);

    if (!clazz->is_array()) {
        // TODO java_lang_InvalidClassException
        throw std::runtime_error("TODO: java_lang_InvalidClassException");
    } else if (clazz->array_element_type->name() == "boolean" || clazz->array_element_type->name() == "byte") {
        // TODO ???
        return sizeof(s1);
    } else if (clazz->array_element_type->name() == "char") {
        return sizeof(u2);
    } else if (clazz->array_element_type->name() == "short") {
        return sizeof(s2);
    } else if (clazz->array_element_type->name() == "int") {
        return sizeof(s4);
    } else if (clazz->array_element_type->name() == "long") {
        return sizeof(s8);
    } else if (clazz->array_element_type->name() == "float") {
        return sizeof(float);
    } else if (clazz->array_element_type->name() == "double") {
        return sizeof(double);
    } else {
        // object
        return sizeof(Reference);
    }
}

JNIEXPORT jboolean JNICALL
static
Unsafe_CompareAndSetObject(JNIEnv *env, jobject unsafe, jobject obj, jlong offset, jobject expected, jobject desired) {
    // TODO "volatile semantics"
    auto *field = (reinterpret_cast<jobject *>(reinterpret_cast<char *>(obj) + offset));
    return __atomic_compare_exchange_n(field, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

JNIEXPORT jboolean JNICALL
static Unsafe_CompareAndSetInt(JNIEnv *env, jobject unsafe, jobject obj, jlong offset, jint expected, jint desired) {
    auto *field = &(reinterpret_cast<Value *>(reinterpret_cast<char *>(obj) + offset))->s4;
    return __atomic_compare_exchange_n(field, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

JNIEXPORT jboolean JNICALL
static Unsafe_CompareAndSetLong(JNIEnv *env, jobject unsafe, jobject obj, jlong offset, jlong expected, jlong desired) {
    auto *field = &(reinterpret_cast<Value *>(reinterpret_cast<char *>(obj) + offset))->s8;
    auto expected_s8 = static_cast<s8>(expected);
    return __atomic_compare_exchange_n(field, &expected_s8, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}


JNIEXPORT jint JNICALL
static Unsafe_AddressSize0(JNIEnv *env, jobject unsafe) {
    return sizeof(void *);
}

JNIEXPORT jboolean JNICALL
static Unsafe_isBigEndian0(JNIEnv *env, jobject unsafe) {
    return std::endian::native == std::endian::big;
}

JNIEXPORT jboolean JNICALL
static Unsafe_unalignedAccess0(JNIEnv *env, jobject unsafe) {
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
        {CC("getObjectVolatile"),   CC("(" OBJ "J)" OBJ ""),          reinterpret_cast<void *>(Unsafe_GetObjectVolatile)},
//        {CC("putObjectVolatile"), CC("(" OBJ "J" OBJ ")V"), Unsafe_PutObjectVolatile},
//
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
        {CC("objectFieldOffset1"),  CC ("(" CLS LANG "String;)J"),    reinterpret_cast<void *>(Unsafe_ObjectFieldOffset1)},
//        {CC("staticFieldOffset0"), CC ("(" FLD ")J"), Unsafe_StaticFieldOffset0},
//        {CC("staticFieldBase0"), CC ("(" FLD ")" OBJ), Unsafe_StaticFieldBase0},
//        {CC("ensureClassInitialized0"), CC ("(" CLS ")V"), Unsafe_EnsureClassInitialized0},
        {CC("arrayBaseOffset0"),    CC ("(" CLS ")I"),                reinterpret_cast<void *>(Unsafe_ArrayBaseOffset0)},
        {CC("arrayIndexScale0"),    CC ("(" CLS ")I"),                reinterpret_cast<void *>(Unsafe_ArrayIndexScale0)},
        {CC("addressSize0"),        CC ("()I"),                       reinterpret_cast<void *>(Unsafe_AddressSize0)},
//        {CC("pageSize"), CC ("()I"), Unsafe_PageSize},
//
//        {CC("defineClass0"), CC ("(" DC_Args ")" CLS), Unsafe_DefineClass0},
//        {CC("allocateInstance"), CC ("(" CLS ")" OBJ), Unsafe_AllocateInstance},
//        {CC("throwException"), CC ("(" THR ")V"), Unsafe_ThrowException},
        {CC("compareAndSetObject"), CC ("(" OBJ "J" OBJ "" OBJ ")Z"), reinterpret_cast<void *>(Unsafe_CompareAndSetObject)},
        {CC("compareAndSetInt"),    CC ("(" OBJ "J""I""I"")Z"),       reinterpret_cast<void *>(Unsafe_CompareAndSetInt)},
        {CC("compareAndSetLong"),   CC ("(" OBJ "J""J""J"")Z"),       reinterpret_cast<void *>(Unsafe_CompareAndSetLong)},
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
//        {CC("loadFence"), CC("()V"), Unsafe_LoadFence},
//        {CC("storeFence"), CC("()V"), Unsafe_StoreFence},
//        {CC("fullFence"), CC("()V"), Unsafe_FullFence},
//
        {CC("isBigEndian0"),        CC("()Z"),                        reinterpret_cast<void *>(Unsafe_isBigEndian0)},
        {CC("unalignedAccess0"),    CC("()Z"),                        reinterpret_cast<void *>(Unsafe_unalignedAccess0)}
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
