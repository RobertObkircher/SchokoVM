#include <cassert>
#include <iostream>
#include <filesystem>
#include <dlfcn.h>

#include "args.hpp"
#include "jni.h"
#include "jvm.h"
#include "interpreter.hpp"

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (!arguments) {
        return 23;
    }

    auto handle = dlopen(arguments->libjava.c_str(), RTLD_LAZY);
    if (auto message = dlerror(); message != nullptr) {
        std::cerr << "!!! " << message << "\n";
        return 1;
    }

    JavaVMInitArgs args;
    args.version = JNI_VERSION_10;
    if (JNI_GetDefaultJavaVMInitArgs(&args) != JNI_OK) {
        throw std::runtime_error("faield to get default options");
    }

    // TODO
    assert(args.options == nullptr);
    JavaVMOption hack[10];
    args.options = hack;
    std::string bootclasspath{"-Xbootclasspath:../jdk/exploded-modules"};
    hack[0].optionString = bootclasspath.data();
    std::string classpath = "-Djava.class.path=" + arguments->classpath;
    hack[1].optionString = classpath.data();
    args.nOptions = 2;

    JavaVM *pvm = nullptr;
    JNIEnv *penv = nullptr;
    jint status = JNI_CreateJavaVM(&pvm, (void **) &penv, &args);
    if (status != JNI_OK) {
        return status;
    }
    Env::set(penv);

    jclass main_class = JVM_FindClassFromBootLoader(penv, arguments->mainclass.c_str());
    if (main_class == nullptr) {
        pvm->DestroyJavaVM();
        throw std::runtime_error("mainclass was not found");
    }

    jmethodID main_method_id = penv->GetStaticMethodID(main_class, "main", "([Ljava/lang/String;)V");
    if (main_method_id == nullptr) {
        pvm->DestroyJavaVM();
        throw std::runtime_error("Couldn't find main method");
    }
    // TODO args
    penv->CallStaticVoidMethod(main_class, main_method_id, Reference{nullptr});

    if (jthrowable exception = penv->ExceptionOccurred(); exception != nullptr) {
        // TODO?
        penv->ExceptionClear();
    }

    // TODO figure out how to free this (or maybe LeakSanitizer won't complain if we just put it in a thread local variable?)
    delete (Thread *) penv->functions->reserved0;
    delete penv->functions;
    delete penv;

    pvm->DestroyJavaVM();
    return 0;
}
