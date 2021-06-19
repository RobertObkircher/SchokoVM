#include <cstdlib>
#include <thread>
#include <iostream>

#include "jvm.h"
#include "classloading.hpp"

// This file was created from the function declarations in jvm.h.
// FIND: (JNICALL\s+)(JVM_[^(\s]*)([^;]*);
// REPLACE: $1$2$3 {
//    UNIMPLEMENTED("$2");
// }

#define UNIMPLEMENTED(x) std::cerr << x; exit(42);
#define LOG(x)

// TODO I have no idea why this is not in the header and why jdk 11 binaries link to it
extern "C" {
JNIEXPORT jboolean JNICALL
JVM_IsUseContainerSupport() {
    UNIMPLEMENTED("JVM_IsUseContainerSupport")
}

JNIEXPORT jboolean JNICALL
JVM_AreNestMates(JNIEnv *env, jclass current, jclass member) {
    UNIMPLEMENTED("JVM_AreNestMates")
}

JNIEXPORT void JNICALL
JVM_InitializeFromArchive(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_InitializeFromArchive")
}

JNIEXPORT jstring JNICALL
JVM_InitClassName(JNIEnv *env, jclass cls) {
    auto *java_class = (ClassFile *) cls;
    auto name = java_class->name();
    std::replace(name.begin(), name.end(), '/', '.');
    auto ref = Heap::get().make_string(name);
    return (jstring) ref.memory;
}

JNIEXPORT jclass JNICALL
JVM_GetNestHost(JNIEnv *env, jclass current) {
    UNIMPLEMENTED("JVM_GetNestHost");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetNestMembers(JNIEnv *env, jclass current) {
    UNIMPLEMENTED("JVM_Clone");
}

}

JNIEXPORT jint JNICALL
JVM_GetInterfaceVersion(void) {
    UNIMPLEMENTED("JVM_GetInterfaceVersion");
}

/*************************************************************************
 PART 1: Functions for Native Libraries
 ************************************************************************/
/*
 * java.lang.Object
 */
JNIEXPORT jint JNICALL
JVM_IHashCode(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("JVM_IHashCode");
}

JNIEXPORT void JNICALL
JVM_MonitorWait(JNIEnv *env, jobject obj, jlong ms) {
    UNIMPLEMENTED("JVM_MonitorWait");
}

JNIEXPORT void JNICALL
JVM_MonitorNotify(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("JVM_MonitorNotify");
}

JNIEXPORT void JNICALL
JVM_MonitorNotifyAll(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("JVM_MonitorNotifyAll");
}

JNIEXPORT jobject JNICALL
JVM_Clone(JNIEnv *env, jobject obj) {
    UNIMPLEMENTED("JVM_Clone");
}

/*
 * java.lang.String
 */
JNIEXPORT jstring JNICALL
JVM_InternString(JNIEnv *env, jstring str) {
    UNIMPLEMENTED("JVM_InternString");
}

/*
 * java.lang.System
 */
JNIEXPORT jlong JNICALL
JVM_CurrentTimeMillis(JNIEnv *env, jclass ignored) {
    UNIMPLEMENTED("JVM_CurrentTimeMillis");
}

JNIEXPORT jlong JNICALL
JVM_NanoTime(JNIEnv *env, jclass ignored) {
    UNIMPLEMENTED("JVM_NanoTime");
}

JNIEXPORT jlong JNICALL
JVM_GetNanoTimeAdjustment(JNIEnv *env, jclass ignored, jlong offset_secs) {
    UNIMPLEMENTED("JVM_GetNanoTimeAdjustment");
}

JNIEXPORT void JNICALL
JVM_ArrayCopy(JNIEnv *env, jclass ignored, jobject src, jint src_pos,
              jobject dst, jint dst_pos, jint length) {
    UNIMPLEMENTED("JVM_ArrayCopy");
}

JNIEXPORT jobject JNICALL
JVM_InitProperties(JNIEnv *env, jobject properties) {
    LOG("JVM_InitProperties");
    // TODO forward default and CLI parameters to the properties object
    return properties;
}


/*
 * java.lang.Runtime
 */
JNIEXPORT void JNICALL
JVM_BeforeHalt() {
    UNIMPLEMENTED("JVM_BeforeHalt");
}

JNIEXPORT void JNICALL
JVM_Halt(jint code) {
    UNIMPLEMENTED("JVM_Halt");
}

JNIEXPORT void JNICALL
JVM_GC(void) {
    UNIMPLEMENTED("JVM_GC");
}

/* Returns the number of real-time milliseconds that have elapsed since the
 * least-recently-inspected heap object was last inspected by the garbage
 * collector.
 *
 * For simple stop-the-world collectors this value is just the time
 * since the most recent collection.  For generational collectors it is the
 * time since the oldest generation was most recently collected.  Other
 * collectors are free to return a pessimistic estimate of the elapsed time, or
 * simply the time since the last full collection was performed.
 *
 * Note that in the presence of reference objects, a given object that is no
 * longer strongly reachable may have to be inspected multiple times before it
 * can be reclaimed.
 */
JNIEXPORT jlong JNICALL
JVM_MaxObjectInspectionAge(void) {
    UNIMPLEMENTED("JVM_MaxObjectInspectionAge");
}

JNIEXPORT jlong JNICALL
JVM_TotalMemory(void) {
    UNIMPLEMENTED("JVM_TotalMemory");
}

JNIEXPORT jlong JNICALL
JVM_FreeMemory(void) {
    UNIMPLEMENTED("JVM_FreeMemory");
}

JNIEXPORT jlong JNICALL
JVM_MaxMemory(void) {
    UNIMPLEMENTED("JVM_MaxMemory");
}

JNIEXPORT jint JNICALL
JVM_ActiveProcessorCount(void) {
    LOG("JVM_ActiveProcessorCount");
    auto hint = std::thread::hardware_concurrency();
    if (hint > 0) {
        return static_cast<jint>(hint);
    } else {
        return 1;
    }
}

JNIEXPORT void *JNICALL
JVM_LoadLibrary(const char *name) {
    UNIMPLEMENTED("JVM_LoadLibrary");
}

JNIEXPORT void JNICALL
JVM_UnloadLibrary(void *handle) {
    UNIMPLEMENTED("JVM_UnloadLibrary");
}

JNIEXPORT void *JNICALL
JVM_FindLibraryEntry(void *handle, const char *name) {
    UNIMPLEMENTED("JVM_FindLibraryEntry");
}

JNIEXPORT jboolean JNICALL
JVM_IsSupportedJNIVersion(jint version) {
    UNIMPLEMENTED("JVM_IsSupportedJNIVersion");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetVmArguments(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetVmArguments");
}


/*
 * java.lang.Throwable
 */
JNIEXPORT void JNICALL
JVM_FillInStackTrace(JNIEnv *env, jobject throwable) {
    UNIMPLEMENTED("JVM_FillInStackTrace");
}

/*
 * java.lang.StackTraceElement
 */
JNIEXPORT void JNICALL
JVM_InitStackTraceElementArray(JNIEnv *env, jobjectArray elements, jobject throwable) {
    UNIMPLEMENTED("JVM_InitStackTraceElementArray");
}

JNIEXPORT void JNICALL
JVM_InitStackTraceElement(JNIEnv *env, jobject element, jobject stackFrameInfo) {
    UNIMPLEMENTED("JVM_InitStackTraceElement");
}

/*
 * java.lang.StackWalker
 */

JNIEXPORT jobject JNICALL
JVM_CallStackWalk(JNIEnv *env, jobject stackStream, jlong mode,
                  jint skip_frames, jint frame_count, jint start_index,
                  jobjectArray frames) {
    UNIMPLEMENTED("JVM_CallStackWalk");
}

JNIEXPORT jint JNICALL
JVM_MoreStackWalk(JNIEnv *env, jobject stackStream, jlong mode, jlong anchor,
                  jint frame_count, jint start_index,
                  jobjectArray frames) {
    UNIMPLEMENTED("JVM_MoreStackWalk");
}

/*
 * java.lang.Thread
 */
JNIEXPORT void JNICALL
JVM_StartThread(JNIEnv *env, jobject thread) {
    UNIMPLEMENTED("JVM_StartThread");
}

JNIEXPORT void JNICALL
JVM_StopThread(JNIEnv *env, jobject thread, jobject exception) {
    UNIMPLEMENTED("JVM_StopThread");
}

JNIEXPORT jboolean JNICALL
JVM_IsThreadAlive(JNIEnv *env, jobject thread) {
    UNIMPLEMENTED("JVM_IsThreadAlive");
}

JNIEXPORT void JNICALL
JVM_SuspendThread(JNIEnv *env, jobject thread) {
    UNIMPLEMENTED("JVM_SuspendThread");
}

JNIEXPORT void JNICALL
JVM_ResumeThread(JNIEnv *env, jobject thread) {
    UNIMPLEMENTED("JVM_ResumeThread");
}

JNIEXPORT void JNICALL
JVM_SetThreadPriority(JNIEnv *env, jobject thread, jint prio) {
    UNIMPLEMENTED("JVM_SetThreadPriority");
}

JNIEXPORT void JNICALL
JVM_Yield(JNIEnv *env, jclass threadClass) {
    UNIMPLEMENTED("JVM_Yield");
}

JNIEXPORT void JNICALL
JVM_Sleep(JNIEnv *env, jclass threadClass, jlong millis) {
    UNIMPLEMENTED("JVM_Sleep");
}

JNIEXPORT jobject JNICALL
JVM_CurrentThread(JNIEnv *env, jclass threadClass) {
    UNIMPLEMENTED("JVM_CurrentThread");
}

JNIEXPORT jint JNICALL
JVM_CountStackFrames(JNIEnv *env, jobject thread) {
    UNIMPLEMENTED("JVM_CountStackFrames");
}

JNIEXPORT void JNICALL
JVM_Interrupt(JNIEnv *env, jobject thread) {
    UNIMPLEMENTED("JVM_Interrupt");
}

JNIEXPORT jboolean JNICALL
JVM_IsInterrupted(JNIEnv *env, jobject thread, jboolean clearInterrupted) {
    UNIMPLEMENTED("JVM_IsInterrupted");
}

JNIEXPORT jboolean JNICALL
JVM_HoldsLock(JNIEnv *env, jclass threadClass, jobject obj) {
    UNIMPLEMENTED("JVM_HoldsLock");
}

JNIEXPORT void JNICALL
JVM_DumpAllStacks(JNIEnv *env, jclass unused) {
    UNIMPLEMENTED("JVM_DumpAllStacks");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetAllThreads(JNIEnv *env, jclass dummy) {
    UNIMPLEMENTED("JVM_GetAllThreads");
}

JNIEXPORT void JNICALL
JVM_SetNativeThreadName(JNIEnv *env, jobject jthread, jstring name) {
    UNIMPLEMENTED("JVM_SetNativeThreadName");
}

/* getStackTrace() and getAllStackTraces() method */
JNIEXPORT jobjectArray JNICALL
JVM_DumpThreads(JNIEnv *env, jclass threadClass, jobjectArray threads) {
    UNIMPLEMENTED("JVM_DumpThreads");
}

/*
 * java.lang.SecurityManager
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetClassContext(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetClassContext");
}

/*
 * java.lang.Package
 */
JNIEXPORT jstring JNICALL
JVM_GetSystemPackage(JNIEnv *env, jstring name) {
    UNIMPLEMENTED("JVM_GetSystemPackage");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetSystemPackages(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetSystemPackages");
}

/*
 * java.lang.ref.Reference
 */
JNIEXPORT jobject JNICALL
JVM_GetAndClearReferencePendingList(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetAndClearReferencePendingList");
}

JNIEXPORT jboolean JNICALL
JVM_HasReferencePendingList(JNIEnv *env) {
    UNIMPLEMENTED("JVM_HasReferencePendingList");
}

JNIEXPORT void JNICALL
JVM_WaitForReferencePendingList(JNIEnv *env) {
    UNIMPLEMENTED("JVM_WaitForReferencePendingList");
}

/*
 * java.io.ObjectInputStream
 */
JNIEXPORT jobject JNICALL
JVM_LatestUserDefinedLoader(JNIEnv *env) {
    UNIMPLEMENTED("JVM_LatestUserDefinedLoader");
}

/*
 * java.lang.reflect.Array
 */
JNIEXPORT jint JNICALL
JVM_GetArrayLength(JNIEnv *env, jobject arr) {
    UNIMPLEMENTED("JVM_GetArrayLength");
}

JNIEXPORT jobject JNICALL
JVM_GetArrayElement(JNIEnv *env, jobject arr, jint index) {
    UNIMPLEMENTED("JVM_GetArrayElement");
}

JNIEXPORT jvalue JNICALL
JVM_GetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index, jint wCode) {
    UNIMPLEMENTED("JVM_GetPrimitiveArrayElement");
}

JNIEXPORT void JNICALL
JVM_SetArrayElement(JNIEnv *env, jobject arr, jint index, jobject val) {
    UNIMPLEMENTED("JVM_SetArrayElement");
}

JNIEXPORT void JNICALL
JVM_SetPrimitiveArrayElement(JNIEnv *env, jobject arr, jint index, jvalue v,
                             unsigned char vCode) {
    UNIMPLEMENTED("JVM_SetPrimitiveArrayElement");
}

JNIEXPORT jobject JNICALL
JVM_NewArray(JNIEnv *env, jclass eltClass, jint length) {
    LOG("JVM_NewArray");
    auto element_class = reinterpret_cast<ClassFile *>(eltClass);
    ClassFile *array_class;
    if (element_class->name() == "boolean" || element_class->name() == "byte") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Byte).array;
    } else if (element_class->name() == "char") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Char).array;
    } else if (element_class->name() == "short") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Short).array;
    } else if (element_class->name() == "int") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Int).array;
    } else if (element_class->name() == "long") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Long).array;
    } else if (element_class->name() == "float") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Float).array;
    } else if (element_class->name() == "double") {
        array_class = BootstrapClassLoader::primitive(Primitive::Type::Double).array;
    } else {
        // object
        array_class = BootstrapClassLoader::get().load(element_class->as_array_element());
    }

    auto reference = Heap::get().new_array<Reference>(array_class, length);
    return reinterpret_cast<jobject>(reference.memory);
}

JNIEXPORT jobject JNICALL
JVM_NewMultiArray(JNIEnv *env, jclass eltClass, jintArray dim) {
    UNIMPLEMENTED("JVM_NewMultiArray");
}


/*
 * Returns the immediate caller class of the native method invoking
 * JVM_GetCallerClass.  The Method.invoke and other frames due to
 * reflection machinery are skipped.
 *
 * The caller is expected to be marked with
 * jdk.internal.reflect.CallerSensitive. The JVM will throw an
 * error if it is not marked properly.
 */
JNIEXPORT jclass JNICALL
JVM_GetCallerClass(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetCallerClass");
}


/*
 * Find primitive classes
 * utf: class name
 */
JNIEXPORT jclass JNICALL
JVM_FindPrimitiveClass(JNIEnv *env, const char *utf) {
    LOG("JVM_FindPrimitiveClass");
    const auto primitives = BootstrapClassLoader::constants().primitives;

    for (size_t i = 0; i < Primitive::TYPE_COUNT; i++) {
        const auto primitive = primitives[i];
        if (std::strcmp(primitive.primitive_name, utf) == 0) {
            return reinterpret_cast<jclass>(primitive.primitive);
        }
    }

    throw std::runtime_error("Coudln't find primitive class " + std::string(utf));
}


/*
 * Find a class from a boot class loader. Returns NULL if class not found.
 */
JNIEXPORT jclass JNICALL
JVM_FindClassFromBootLoader(JNIEnv *env, const char *name) {
    LOG("JVM_FindClassFromBootLoader");
    std::string n(name);
    ClassFile *clazz = BootstrapClassLoader::get().load(n);
    return reinterpret_cast<jclass>(clazz);
}

/*
 * Find a class from a given class loader.  Throws ClassNotFoundException.
 *  name:   name of class
 *  init:   whether initialization is done
 *  loader: class loader to look up the class. This may not be the same as the caller's
 *          class loader.
 *  caller: initiating class. The initiating class may be null when a security
 *          manager is not installed.
 */
JNIEXPORT jclass JNICALL
JVM_FindClassFromCaller(JNIEnv *env, const char *name, jboolean init,
                        jobject loader, jclass caller) {
    UNIMPLEMENTED("JVM_FindClassFromCaller");
}

/*
 * Find a class from a given class.
 */
JNIEXPORT jclass JNICALL
JVM_FindClassFromClass(JNIEnv *env, const char *name, jboolean init,
                       jclass from) {
    UNIMPLEMENTED("JVM_FindClassFromClass");
}

/* Find a loaded class cached by the VM */
JNIEXPORT jclass JNICALL
JVM_FindLoadedClass(JNIEnv *env, jobject loader, jstring name) {
    UNIMPLEMENTED("JVM_FindLoadedClass");
}

/* Define a class */
JNIEXPORT jclass JNICALL
JVM_DefineClass(JNIEnv *env, const char *name, jobject loader, const jbyte *buf,
                jsize len, jobject pd) {
    UNIMPLEMENTED("JVM_DefineClass");
}

/* Define a class with a source (added in JDK1.5) */
JNIEXPORT jclass JNICALL
JVM_DefineClassWithSource(JNIEnv *env, const char *name, jobject loader,
                          const jbyte *buf, jsize len, jobject pd,
                          const char *source) {
    UNIMPLEMENTED("JVM_DefineClassWithSource");
}

/*
 * Module support funcions
 */

/*
 * Define a module with the specified packages and bind the module to the
 * given class loader.
 *  module:       module to define
 *  is_open:      specifies if module is open (currently ignored)
 *  version:      the module version
 *  location:     the module location
 *  packages:     list of packages in the module
 *  num_packages: number of packages in the module
 */
JNIEXPORT void JNICALL
JVM_DefineModule(JNIEnv *env, jobject module, jboolean is_open, jstring version,
                 jstring location, const char *const *packages, jsize num_packages) {
    UNIMPLEMENTED("JVM_DefineModule");
}

/*
 * Set the boot loader's unnamed module.
 *  module: boot loader's unnamed module
 */
JNIEXPORT void JNICALL
JVM_SetBootLoaderUnnamedModule(JNIEnv *env, jobject module) {
    UNIMPLEMENTED("JVM_SetBootLoaderUnnamedModule");
}

/*
 * Do a qualified export of a package.
 *  from_module: module containing the package to export
 *  package:     name of the package to export
 *  to_module:   module to export the package to
 */
JNIEXPORT void JNICALL
JVM_AddModuleExports(JNIEnv *env, jobject from_module, const char *package, jobject to_module) {
    UNIMPLEMENTED("JVM_AddModuleExports");
}

/*
 * Do an export of a package to all unnamed modules.
 *  from_module: module containing the package to export
 *  package:     name of the package to export to all unnamed modules
 */
JNIEXPORT void JNICALL
JVM_AddModuleExportsToAllUnnamed(JNIEnv *env, jobject from_module, const char *package) {
    UNIMPLEMENTED("JVM_AddModuleExportsToAllUnnamed");
}

/*
 * Do an unqualified export of a package.
 *  from_module: module containing the package to export
 *  package:     name of the package to export
 */
JNIEXPORT void JNICALL
JVM_AddModuleExportsToAll(JNIEnv *env, jobject from_module, const char *package) {
    UNIMPLEMENTED("JVM_AddModuleExportsToAll");
}

/*
 * Add a module to the list of modules that a given module can read.
 *  from_module:   module requesting read access
 *  source_module: module that from_module wants to read
 */
JNIEXPORT void JNICALL
JVM_AddReadsModule(JNIEnv *env, jobject from_module, jobject source_module) {
    UNIMPLEMENTED("JVM_AddReadsModule");
}

/*
 * Reflection support functions
 */

JNIEXPORT jstring JNICALL
JVM_GetClassName(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassName");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetClassInterfaces(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassInterfaces");
}

JNIEXPORT jboolean JNICALL
JVM_IsInterface(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_IsInterface");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetClassSigners(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassSigners");
}

JNIEXPORT void JNICALL
JVM_SetClassSigners(JNIEnv *env, jclass cls, jobjectArray signers) {
    UNIMPLEMENTED("JVM_SetClassSigners");
}

JNIEXPORT jobject JNICALL
JVM_GetProtectionDomain(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetProtectionDomain");
}

JNIEXPORT jboolean JNICALL
JVM_IsArrayClass(JNIEnv *env, jclass cls) {
    LOG("JVM_IsArrayClass");
    auto clazz = reinterpret_cast<ClassFile *>(cls);
    return clazz->is_array();
}

JNIEXPORT jboolean JNICALL
JVM_IsPrimitiveClass(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_IsPrimitiveClass");
}

JNIEXPORT jint JNICALL
JVM_GetClassModifiers(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassModifiers");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetDeclaredClasses(JNIEnv *env, jclass ofClass) {
    UNIMPLEMENTED("JVM_GetDeclaredClasses");
}

JNIEXPORT jclass JNICALL
JVM_GetDeclaringClass(JNIEnv *env, jclass ofClass) {
    UNIMPLEMENTED("JVM_GetDeclaringClass");
}

JNIEXPORT jstring JNICALL
JVM_GetSimpleBinaryName(JNIEnv *env, jclass ofClass) {
    UNIMPLEMENTED("JVM_GetSimpleBinaryName");
}

/* Generics support (JDK 1.5) */
JNIEXPORT jstring JNICALL
JVM_GetClassSignature(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassSignature");
}

/* Annotations support (JDK 1.5) */
JNIEXPORT jbyteArray JNICALL
JVM_GetClassAnnotations(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassAnnotations");
}

/* Type use annotations support (JDK 1.8) */

JNIEXPORT jbyteArray JNICALL
JVM_GetClassTypeAnnotations(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassTypeAnnotations");
}

JNIEXPORT jbyteArray JNICALL
JVM_GetFieldTypeAnnotations(JNIEnv *env, jobject field) {
    UNIMPLEMENTED("JVM_GetFieldTypeAnnotations");
}

JNIEXPORT jbyteArray JNICALL
JVM_GetMethodTypeAnnotations(JNIEnv *env, jobject method) {
    UNIMPLEMENTED("JVM_GetMethodTypeAnnotations");
}

/*
 * New (JDK 1.4) reflection implementation
 */

JNIEXPORT jobjectArray JNICALL
JVM_GetClassDeclaredMethods(JNIEnv *env, jclass ofClass, jboolean publicOnly) {
    UNIMPLEMENTED("JVM_GetClassDeclaredMethods");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetClassDeclaredFields(JNIEnv *env, jclass ofClass, jboolean publicOnly) {
    UNIMPLEMENTED("JVM_GetClassDeclaredFields");
}

JNIEXPORT jobjectArray JNICALL
JVM_GetClassDeclaredConstructors(JNIEnv *env, jclass ofClass, jboolean publicOnly) {
    UNIMPLEMENTED("JVM_GetClassDeclaredConstructors");
}

/* Differs from JVM_GetClassModifiers in treatment of inner classes.
   This returns the access flags for the class as specified in the
   class file rather than searching the InnerClasses attribute (if
   present) to find the source-level access flags. Only the values of
   the low 13 bits (i.e., a mask of 0x1FFF) are guaranteed to be
   valid. */
JNIEXPORT jint JNICALL
JVM_GetClassAccessFlags(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassAccessFlags");
}

/* The following two reflection routines are still needed due to startup time issues */
/*
 * java.lang.reflect.Method
 */
JNIEXPORT jobject JNICALL
JVM_InvokeMethod(JNIEnv *env, jobject method, jobject obj, jobjectArray args0) {
    UNIMPLEMENTED("JVM_InvokeMethod");
}

/*
 * java.lang.reflect.Constructor
 */
JNIEXPORT jobject JNICALL
JVM_NewInstanceFromConstructor(JNIEnv *env, jobject c, jobjectArray args0) {
    UNIMPLEMENTED("JVM_NewInstanceFromConstructor");
}

/*
 * Constant pool access; currently used to implement reflective access to annotations (JDK 1.5)
 */

JNIEXPORT jobject JNICALL
JVM_GetClassConstantPool(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetClassConstantPool");
}

JNIEXPORT jint JNICALL JVM_ConstantPoolGetSize
        (JNIEnv *env, jobject unused, jobject jcpool) {
    UNIMPLEMENTED("JVM_ConstantPoolGetSize");
}

JNIEXPORT jclass JNICALL JVM_ConstantPoolGetClassAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetClassAt");
}

JNIEXPORT jclass JNICALL JVM_ConstantPoolGetClassAtIfLoaded
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetClassAtIfLoaded");
}

JNIEXPORT jint JNICALL JVM_ConstantPoolGetClassRefIndexAt
        (JNIEnv *env, jobject obj, jobject unused, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetClassRefIndexAt");
}

JNIEXPORT jobject JNICALL JVM_ConstantPoolGetMethodAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetMethodAt");
}

JNIEXPORT jobject JNICALL JVM_ConstantPoolGetMethodAtIfLoaded
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetMethodAtIfLoaded");
}

JNIEXPORT jobject JNICALL JVM_ConstantPoolGetFieldAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetFieldAt");
}

JNIEXPORT jobject JNICALL JVM_ConstantPoolGetFieldAtIfLoaded
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetFieldAtIfLoaded");
}

JNIEXPORT jobjectArray JNICALL JVM_ConstantPoolGetMemberRefInfoAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetMemberRefInfoAt");
}

JNIEXPORT jint JNICALL JVM_ConstantPoolGetNameAndTypeRefIndexAt
        (JNIEnv *env, jobject obj, jobject unused, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetNameAndTypeRefIndexAt");
}

JNIEXPORT jobjectArray JNICALL JVM_ConstantPoolGetNameAndTypeRefInfoAt
        (JNIEnv *env, jobject obj, jobject unused, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetNameAndTypeRefInfoAt");
}

JNIEXPORT jint JNICALL JVM_ConstantPoolGetIntAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetIntAt");
}

JNIEXPORT jlong JNICALL JVM_ConstantPoolGetLongAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetLongAt");
}

JNIEXPORT jfloat JNICALL JVM_ConstantPoolGetFloatAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetFloatAt");
}

JNIEXPORT jdouble JNICALL JVM_ConstantPoolGetDoubleAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetDoubleAt");
}

JNIEXPORT jstring JNICALL JVM_ConstantPoolGetStringAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetStringAt");
}

JNIEXPORT jstring JNICALL JVM_ConstantPoolGetUTF8At
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetUTF8At");
}

JNIEXPORT jbyte JNICALL JVM_ConstantPoolGetTagAt
        (JNIEnv *env, jobject unused, jobject jcpool, jint index) {
    UNIMPLEMENTED("JVM_ConstantPoolGetTagAt");
}

/*
 * Parameter reflection
 */

JNIEXPORT jobjectArray JNICALL
JVM_GetMethodParameters(JNIEnv *env, jobject method) {
    UNIMPLEMENTED("JVM_GetMethodParameters");
}

/*
 * java.security.*
 */

JNIEXPORT jobject JNICALL
JVM_DoPrivileged(JNIEnv *env, jclass cls,
                 jobject action, jobject context, jboolean wrapException) {
    UNIMPLEMENTED("JVM_DoPrivileged");
}

JNIEXPORT jobject JNICALL
JVM_GetInheritedAccessControlContext(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetInheritedAccessControlContext");
}

JNIEXPORT jobject JNICALL
JVM_GetStackAccessControlContext(JNIEnv *env, jclass cls) {
    UNIMPLEMENTED("JVM_GetStackAccessControlContext");
}

/*
 * Signal support, used to implement the shutdown sequence.  Every VM must
 * support JVM_SIGINT and JVM_SIGTERM, raising the former for user interrupts
 * (^C) and the latter for external termination (kill, system shutdown, etc.).
 * Other platform-dependent signal values may also be supported.
 */

JNIEXPORT void *JNICALL
JVM_RegisterSignal(jint sig, void *handler) {
    UNIMPLEMENTED("JVM_RegisterSignal");
}

JNIEXPORT jboolean JNICALL
JVM_RaiseSignal(jint sig) {
    UNIMPLEMENTED("JVM_RaiseSignal");
}

JNIEXPORT jint JNICALL
JVM_FindSignal(const char *name) {
    UNIMPLEMENTED("JVM_FindSignal");
}

/*
 * Retrieve the assertion directives for the specified class.
 */
JNIEXPORT jboolean JNICALL
JVM_DesiredAssertionStatus(JNIEnv *env, jclass unused, jclass cls) {
    LOG("JVM_DesiredAssertionStatus");
    return false; // TODO assertions
}

/*
 * Retrieve the assertion directives from the VM.
 */
JNIEXPORT jobject JNICALL
JVM_AssertionStatusDirectives(JNIEnv *env, jclass unused) {
    UNIMPLEMENTED("JVM_AssertionStatusDirectives");
}

/*
 * java.util.concurrent.atomic.AtomicLong
 */
JNIEXPORT jboolean JNICALL
JVM_SupportsCX8(void) {
    UNIMPLEMENTED("JVM_SupportsCX8");
}

/*
 * com.sun.dtrace.jsdt support
 */

/*
 * Get the version number the JVM was built with
 */
JNIEXPORT jint JNICALL
JVM_DTraceGetVersion(JNIEnv *env) {
    UNIMPLEMENTED("JVM_DTraceGetVersion");
}

/*
 * Register new probe with given signature, return global handle
 *
 * The version passed in is the version that the library code was
 * built with.
 */
JNIEXPORT jlong JNICALL
JVM_DTraceActivate(JNIEnv *env, jint version, jstring module_name,
                   jint providers_count, JVM_DTraceProvider *providers) {
    UNIMPLEMENTED("JVM_DTraceActivate");
}

/*
 * Check JSDT probe
 */
JNIEXPORT jboolean JNICALL
JVM_DTraceIsProbeEnabled(JNIEnv *env, jmethodID method) {
    UNIMPLEMENTED("JVM_DTraceIsProbeEnabled");
}

/*
 * Destroy custom DOF
 */
JNIEXPORT void JNICALL
JVM_DTraceDispose(JNIEnv *env, jlong activation_handle) {
    UNIMPLEMENTED("JVM_DTraceDispose");
}

/*
 * Check to see if DTrace is supported by OS
 */
JNIEXPORT jboolean JNICALL
JVM_DTraceIsSupported(JNIEnv *env) {
    UNIMPLEMENTED("JVM_DTraceIsSupported");
}

/*************************************************************************
 PART 2: Support for the Verifier and Class File Format Checker
 ************************************************************************/
/*
 * Return the class name in UTF format. The result is valid
 * until JVM_ReleaseUTf is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetClassNameUTF(JNIEnv *env, jclass cb) {
    UNIMPLEMENTED("JVM_GetClassNameUTF");
}

/*
 * Returns the constant pool types in the buffer provided by "types."
 */
JNIEXPORT void JNICALL
JVM_GetClassCPTypes(JNIEnv *env, jclass cb, unsigned char *types) {
    UNIMPLEMENTED("JVM_GetClassCPTypes");
}

/*
 * Returns the number of Constant Pool entries.
 */
JNIEXPORT jint JNICALL
JVM_GetClassCPEntriesCount(JNIEnv *env, jclass cb) {
    UNIMPLEMENTED("JVM_GetClassCPEntriesCount");
}

/*
 * Returns the number of *declared* fields or methods.
 */
JNIEXPORT jint JNICALL
JVM_GetClassFieldsCount(JNIEnv *env, jclass cb) {
    UNIMPLEMENTED("JVM_GetClassFieldsCount");
}

JNIEXPORT jint JNICALL
JVM_GetClassMethodsCount(JNIEnv *env, jclass cb) {
    UNIMPLEMENTED("JVM_GetClassMethodsCount");
}

/*
 * Returns the CP indexes of exceptions raised by a given method.
 * Places the result in the given buffer.
 *
 * The method is identified by method_index.
 */
JNIEXPORT void JNICALL
JVM_GetMethodIxExceptionIndexes(JNIEnv *env, jclass cb, jint method_index,
                                unsigned short *exceptions) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionIndexes");
}
/*
 * Returns the number of exceptions raised by a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxExceptionsCount(JNIEnv *env, jclass cb, jint method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionsCount");
}

/*
 * Returns the byte code sequence of a given method.
 * Places the result in the given buffer.
 *
 * The method is identified by method_index.
 */
JNIEXPORT void JNICALL
JVM_GetMethodIxByteCode(JNIEnv *env, jclass cb, jint method_index,
                        unsigned char *code) {
    UNIMPLEMENTED("JVM_GetMethodIxByteCode");
}

/*
 * Returns the length of the byte code sequence of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxByteCodeLength(JNIEnv *env, jclass cb, jint method_index) {
    UNIMPLEMENTED("JVM_GetMethodIxByteCodeLength");
}

/*
 * Returns the exception table entry at entry_index of a given method.
 * Places the result in the given buffer.
 *
 * The method is identified by method_index.
 */
JNIEXPORT void JNICALL
JVM_GetMethodIxExceptionTableEntry(JNIEnv *env, jclass cb, jint method_index,
                                   jint entry_index,
                                   JVM_ExceptionTableEntryType *entry) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionTableEntry");
}

/*
 * Returns the length of the exception table of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxExceptionTableLength(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_GetMethodIxExceptionTableLength");
}

/*
 * Returns the modifiers of a given field.
 * The field is identified by field_index.
 */
JNIEXPORT jint JNICALL
JVM_GetFieldIxModifiers(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_GetFieldIxModifiers");
}

/*
 * Returns the modifiers of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxModifiers(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_GetMethodIxModifiers");
}

/*
 * Returns the number of local variables of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxLocalsCount(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_GetMethodIxLocalsCount");
}

/*
 * Returns the number of arguments (including this pointer) of a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxArgsSize(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_GetMethodIxArgsSize");
}

/*
 * Returns the maximum amount of stack (in words) used by a given method.
 * The method is identified by method_index.
 */
JNIEXPORT jint JNICALL
JVM_GetMethodIxMaxStack(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_GetMethodIxMaxStack");
}

/*
 * Is a given method a constructor.
 * The method is identified by method_index.
 */
JNIEXPORT jboolean JNICALL
JVM_IsConstructorIx(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_IsConstructorIx");
}

/*
 * Is the given method generated by the VM.
 * The method is identified by method_index.
 */
JNIEXPORT jboolean JNICALL
JVM_IsVMGeneratedMethodIx(JNIEnv *env, jclass cb, int index) {
    UNIMPLEMENTED("JVM_IsVMGeneratedMethodIx");
}

/*
 * Returns the name of a given method in UTF format.
 * The result remains valid until JVM_ReleaseUTF is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetMethodIxNameUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetMethodIxNameUTF");
}

/*
 * Returns the signature of a given method in UTF format.
 * The result remains valid until JVM_ReleaseUTF is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetMethodIxSignatureUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetMethodIxSignatureUTF");
}

/*
 * Returns the name of the field referred to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPFieldNameUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPFieldNameUTF");
}

/*
 * Returns the name of the method referred to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPMethodNameUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPMethodNameUTF");
}

/*
 * Returns the signature of the method referred to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPMethodSignatureUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPMethodSignatureUTF");
}

/*
 * Returns the signature of the field referred to at a given constant pool
 * index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPFieldSignatureUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPFieldSignatureUTF");
}

/*
 * Returns the class name referred to at a given constant pool index.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPClassNameUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPClassNameUTF");
}

/*
 * Returns the class name referred to at a given constant pool index.
 *
 * The constant pool entry must refer to a CONSTANT_Fieldref.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPFieldClassNameUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPFieldClassNameUTF");
}

/*
 * Returns the class name referred to at a given constant pool index.
 *
 * The constant pool entry must refer to CONSTANT_Methodref or
 * CONSTANT_InterfaceMethodref.
 *
 * The result is in UTF format and remains valid until JVM_ReleaseUTF
 * is called.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 */
JNIEXPORT const char *JNICALL
JVM_GetCPMethodClassNameUTF(JNIEnv *env, jclass cb, jint index) {
    UNIMPLEMENTED("JVM_GetCPMethodClassNameUTF");
}

/*
 * Returns the modifiers of a field in calledClass. The field is
 * referred to in class cb at constant pool entry index.
 *
 * The caller must treat the string as a constant and not modify it
 * in any way.
 *
 * Returns -1 if the field does not exist in calledClass.
 */
JNIEXPORT jint JNICALL
JVM_GetCPFieldModifiers(JNIEnv *env, jclass cb, int index, jclass calledClass) {
    UNIMPLEMENTED("JVM_GetCPFieldModifiers");
}

/*
 * Returns the modifiers of a method in calledClass. The method is
 * referred to in class cb at constant pool entry index.
 *
 * Returns -1 if the method does not exist in calledClass.
 */
JNIEXPORT jint JNICALL
JVM_GetCPMethodModifiers(JNIEnv *env, jclass cb, int index, jclass calledClass) {
    UNIMPLEMENTED("JVM_GetCPMethodModifiers");
}

/*
 * Releases the UTF string obtained from the VM.
 */
JNIEXPORT void JNICALL
JVM_ReleaseUTF(const char *utf) {
    UNIMPLEMENTED("JVM_ReleaseUTF");
}

/*
 * Compare if two classes are in the same package.
 */
JNIEXPORT jboolean JNICALL
JVM_IsSameClassPackage(JNIEnv *env, jclass class1, jclass class2) {
    UNIMPLEMENTED("JVM_IsSameClassPackage");
}

/*************************************************************************
 PART 3: I/O and Network Support
 ************************************************************************/

/*
 * Convert a pathname into native format.  This function does syntactic
 * cleanup, such as removing redundant separator characters.  It modifies
 * the given pathname string in place.
 */
JNIEXPORT char *JNICALL
JVM_NativePath(char *) {
    UNIMPLEMENTED("JVM_NativePath");
}

/*
 * The standard printing functions supported by the Java VM. (Should they
 * be renamed to JVM_* in the future?
 */

/* jio_snprintf() and jio_vsnprintf() behave like snprintf(3) and vsnprintf(3),
 *  respectively, with the following differences:
 * - The string written to str is always zero-terminated, also in case of
 *   truncation (count is too small to hold the result string), unless count
 *   is 0. In case of truncation count-1 characters are written and '\0'
 *   appendend.
 * - If count is too small to hold the whole string, -1 is returned across
 *   all platforms. */

JNIEXPORT int
jio_vsnprintf(char *str, size_t count, const char *fmt, va_list args) {
    // Reject count values that are negative signed values converted to
    // unsigned; see bug 4399518, 4417214
    if ((intptr_t) count <= 0) return -1;

    int result = vsnprintf(str, count, fmt, args);
    if (result > 0 && (size_t) result >= count) {
        str[count - 1] = 0;
        result = -1;
    }

    return result;
}

JNIEXPORT int
jio_snprintf(char *str, size_t count, const char *fmt, ...) {
    UNIMPLEMENTED("jio_snprintf");
}

JNIEXPORT int
jio_fprintf(FILE *, const char *fmt, ...) {
    UNIMPLEMENTED("jio_fprintf");
}

JNIEXPORT int
jio_vfprintf(FILE *, const char *fmt, va_list args) {
    UNIMPLEMENTED("jio_vfprintf");
}


JNIEXPORT void *JNICALL
JVM_RawMonitorCreate(void) {
    UNIMPLEMENTED("JVM_RawMonitorCreate");
}

JNIEXPORT void JNICALL
JVM_RawMonitorDestroy(void *mon) {
    UNIMPLEMENTED("JVM_RawMonitorDestroy");
}

JNIEXPORT jint JNICALL
JVM_RawMonitorEnter(void *mon) {
    UNIMPLEMENTED("JVM_RawMonitorEnter");
}

JNIEXPORT void JNICALL
JVM_RawMonitorExit(void *mon) {
    UNIMPLEMENTED("JVM_RawMonitorExit");
}

/*
 * java.lang.management support
 */
JNIEXPORT void *JNICALL
JVM_GetManagement(jint version) {
    UNIMPLEMENTED("JVM_GetManagement");
}

/*
 * com.sun.tools.attach.VirtualMachine support
 *
 * Initialize the agent properties with the properties maintained in the VM.
 */
JNIEXPORT jobject JNICALL
JVM_InitAgentProperties(JNIEnv *env, jobject agent_props) {
    UNIMPLEMENTED("JVM_InitAgentProperties");
}

JNIEXPORT jstring JNICALL
JVM_GetTemporaryDirectory(JNIEnv *env) {
    UNIMPLEMENTED("JVM_GetTemporaryDirectory");
}

/* Generics reflection support.
 *
 * Returns information about the given class's EnclosingMethod
 * attribute, if present, or null if the class had no enclosing
 * method.
 *
 * If non-null, the returned array contains three elements. Element 0
 * is the java.lang.Class of which the enclosing method is a member,
 * and elements 1 and 2 are the java.lang.Strings for the enclosing
 * method's name and descriptor, respectively.
 */
JNIEXPORT jobjectArray JNICALL
JVM_GetEnclosingMethodInfo(JNIEnv *env, jclass ofClass) {
    UNIMPLEMENTED("JVM_GetEnclosingMethodInfo");
}

JNIEXPORT void JNICALL
JVM_GetVersionInfo(JNIEnv *env, jvm_version_info *info, size_t info_size) {
    UNIMPLEMENTED("JVM_GetVersionInfo");
}
