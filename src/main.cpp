#include <iostream>
#include <filesystem>
#include <unordered_map>

#include "args.hpp"
#include "classfile.hpp"
#include "zip.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "util.hpp"

int run(const Arguments &arguments) {
    std::vector<ClassFile> class_files_list;

    {
        std::vector<char> buffer;
        buffer.resize(1024);

        for (auto &path : split(arguments.classpath, ':')) {
            if (path.ends_with(".jar") || path.ends_with(".zip")) {
                try {
                    read_entire_jar(path.c_str(), buffer, class_files_list);
                } catch (ZipException &e) {
                    std::cerr << "Could not read jar file " << path << " error: " << e.what() << "\n";
                    return 23;
                }
            } else {
                for (const auto &directory_entry : std::filesystem::recursive_directory_iterator(path)) {
                    if (directory_entry.is_regular_file() && directory_entry.path().extension() == ".class") {
                        std::ifstream in{directory_entry.path(), std::ios::in | std::ios::binary};
                        if (in) {
                            Parser parser{in};
                            class_files_list.push_back(parser.parse());
                        } else {
                            std::cerr << "Failed to read file\n";
                            return -2;
                        }
                    }
                }
            }
        }
    }

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