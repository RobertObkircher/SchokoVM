#include <cassert>
#include <iostream>
#include <filesystem>
#include <locale>
#include <codecvt>

#include "args.hpp"
#include "classloading.hpp"
#include "jni.h"
#include "jvm.h"
#include "interpreter.hpp"

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (!arguments) {
        return 23;
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
    std::string bootclasspath{"-Xbootclasspath:../jdk/exploded-modules/java.base"};
    hack[0].optionString = bootclasspath.data();
    std::string classpath = "-Djava.class.path=" + arguments->classpath;
    hack[1].optionString = classpath.data();
    std::string javahome = "-Xjavahome:" + arguments->java_home;
    hack[2].optionString = javahome.data();
    args.nOptions = 3;

    JavaVM *pvm = nullptr;
    JNIEnv *penv = nullptr;
    jint status = JNI_CreateJavaVM(&pvm, (void **) &penv, &args);
    if (status != JNI_OK) {
        return status;
    }

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
    auto args_array = penv->NewObjectArray(static_cast<jsize>(arguments->remaining.size()),
                                           reinterpret_cast<jclass>(BootstrapClassLoader::constants().java_lang_String),
                                           nullptr);
    if (penv->ExceptionCheck()) {
        pvm->DestroyJavaVM();
        throw std::runtime_error("Couldn't find create args array");
    }
    for (size_t i = 0; i < arguments->remaining.size(); i++) {
        const auto &str = arguments->remaining[i];
        auto str_utf16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(str);
        auto str_obj = penv->NewString(reinterpret_cast<const jchar *>(str_utf16.c_str()),
                                       static_cast<jsize>(str_utf16.length()));
        if (penv->ExceptionCheck()) {
            pvm->DestroyJavaVM();
            throw std::runtime_error("Couldn't create arg string");
        }
        penv->SetObjectArrayElement(args_array, static_cast<jsize>(i), str_obj);
        if (penv->ExceptionCheck()) {
            pvm->DestroyJavaVM();
            throw std::runtime_error("Couldn't set arg string");
        }
    }
    penv->CallStaticVoidMethod(main_class, main_method_id, args_array);

    int exit = 0;
    if (jthrowable exception = penv->ExceptionOccurred(); exception != nullptr) {
        penv->ExceptionClear();

        std::cerr << "Exception in thread \"main\" ";

        ClassFile *clazz = BootstrapClassLoader::constants().java_lang_Throwable;
        jmethodID print_stack_trace = penv->GetMethodID((jclass) clazz, "printStackTrace", "()V");
        if (print_stack_trace == nullptr) {
            pvm->DestroyJavaVM();
            throw std::runtime_error("Couldn't find printStackTrace method");
        }
        penv->CallNonvirtualVoidMethod(exception, (jclass) clazz, print_stack_trace);
        exit = 1;
    }

    pvm->DestroyJavaVM();
    return exit;
}
