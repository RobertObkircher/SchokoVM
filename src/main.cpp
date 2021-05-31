#include <iostream>
#include <filesystem>

#include "args.hpp"
#include "classloading.hpp"
#include "interpreter.hpp"

int run(const Arguments &arguments) {
    // todo: we use the classpath as bootstrap classpath as a hack
    BootstrapClassLoader bootstrap_class_loader{arguments.classpath};

    auto main_class = bootstrap_class_loader.load(arguments.mainclass);
    if (main_class == nullptr) {
        std::cerr << "mainclass was not found!\n";
        return 5;
    }

    return interpret(bootstrap_class_loader, main_class);
}

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (arguments) {
        return run(*arguments);
    } else {
        return 23;
    }
}