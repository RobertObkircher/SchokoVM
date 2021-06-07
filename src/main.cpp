#include <iostream>
#include <filesystem>

#include "args.hpp"
#include "classloading.hpp"
#include "interpreter.hpp"

static const u2 MAIN_ACCESS_FLAGS = (static_cast<u2>(FieldInfoAccessFlags::ACC_PUBLIC) |
                                     static_cast<u2>(FieldInfoAccessFlags::ACC_STATIC));
static const auto MAIN_NAME = "main";
static const auto MAIN_DESCRIPTOR = "([Ljava/lang/String;)V";


int run(const Arguments &arguments) {
    // todo: we use the classpath as bootstrap classpath as a hack
    BootstrapClassLoader::get().initialize_with_boot_classpath(arguments.classpath);

    auto main_class = BootstrapClassLoader::get().load(arguments.mainclass);
    if (main_class == nullptr) {
        std::cerr << "mainclass was not found!\n";
        return 5;
    }

    auto main_method_iter = std::find_if(
            main_class->methods.begin(), main_class->methods.end(),
            [](const method_info &m) {
                return m.name_index->value == MAIN_NAME &&
                       m.descriptor_index->value == MAIN_DESCRIPTOR &&
                       (m.access_flags & MAIN_ACCESS_FLAGS) == MAIN_ACCESS_FLAGS;
            }
    );
    if (main_method_iter == std::end(main_class->methods)) {
        throw std::runtime_error("Couldn't find main method");
    }
    method_info *main_method = &*main_method_iter;

    Thread thread{};
    thread.stack.memory.resize(1024 * 1024 / sizeof(Value)); // 1mb for now

    thread.stack.memory_used = 1;
    thread.stack.memory[0] = Value(0); // TODO args[] for main

    return interpret(thread, main_class, main_method).s4;
}

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (arguments) {
        return run(*arguments);
    } else {
        return 23;
    }
}