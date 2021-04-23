#include <iostream>
#include <unordered_map>

#include "args.hpp"
#include "classfile.hpp"
#include "jar.hpp"
#include "interpreter.hpp"

int run(const Arguments &arguments) {
    std::vector<ClassFile> class_files_list;

    auto classpath = arguments.classpath;

    // TODO split by : and read normal directories
    if ((classpath.find(':') != std::string::npos || classpath.find(".jar") != classpath.length() - 4)) {
        std::cerr << "unimplemented: the classpath must consist of a single jar file";
        return -6;
    }

    {
        std::vector<char> buffer;
        buffer.resize(1024);
        const char *path = classpath.c_str();
        auto error_message = read_entire_jar(path, buffer, class_files_list);
        if (error_message) {
            std::cerr << "Could not read jar file " << path << " error: " << *error_message << "\n";
            return 23;
        }
    }

//    std::ifstream in{classpath, std::ios::in | std::ios::binary};
//    if (in) {
//        Parser parser{in};
//        class_files_list.push_back(parser.parse());
//    } else {
//        std::cerr << "Failed to read file\n";
//        return -2;
//    }

    std::unordered_map<std::string_view, ClassFile *> class_files;
    for (auto &clazz : class_files_list) {
        std::string_view name{clazz.this_class->name->value};
        auto pair = class_files.insert({name, &clazz});
        if (!pair.second) {
            std::cerr << "The class '" << name << "' was found twice!\n";
            return 84;
        }
    }

    auto main_class = class_files.find(arguments.mainclass);
    if (main_class == class_files.end()) {
        std::cerr << "mainclass was not found!\n";
        return 5;
    }

    return interpret(class_files, main_class->second);
}

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (arguments) {
        return run(*arguments);
    } else {
        return 23;
    }
}