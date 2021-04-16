#include <iostream>

#include "args.hpp"
#include "classfile.hpp"
#include "jar.hpp"

int run(const Arguments &arguments) {
    std::vector<ClassFile> class_files;

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
        auto error_message = read_entire_jar(path, buffer, class_files);
        if (error_message) {
            std::cerr << "Could not read jar file " << path << " error: " << *error_message << "\n";
            return 23;
        }
    }

//    std::ifstream in{classpath, std::ios::in | std::ios::binary};
//    if (in) {
//        Parser parser{in};
//        class_files.push_back(parser.parse());
//    } else {
//        std::cerr << "Failed to read file\n";
//        return -2;
//    }

    ssize_t index = -1;
    for (size_t i = 0; i < class_files.size(); ++i) {
        auto &item = class_files[i];
        auto &string_info = item.constant_pool.get<CONSTANT_String_info>(item.this_class);
        auto &name = item.constant_pool.get<CONSTANT_Utf8_info>(string_info.string_index).value;

        if (name == arguments.mainclass) {
            index = static_cast<ssize_t>(i);
        }
    }
    if (index == -1) {
        std::cerr << "mainclass was not found!\n";
        return 5;
    }

    // TODO actually execute java code...
    std::cout << "Hello, world!\n";
    return 0;
}

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (arguments) {
        return run(*arguments);
    } else {
        return 23;
    }
}