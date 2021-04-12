#include <cstring>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "classfile.hpp"
#include "parser.hpp"

class FileDeleter {
    std::unordered_set<std::string> paths_to_delete;

public:
    virtual ~FileDeleter() {
        delete_files();
    }

public:
    void add(const std::string &path) {
        paths_to_delete.insert(path);
    }

    void delete_files() {
        for (const auto &path : paths_to_delete) {
            if (remove(path.c_str()) != 0) {
                std::cerr << "failed to remove " << path << "\n";
            }
        }
        paths_to_delete.clear();
    }
};

int main(int argc, char *argv[]) {
    auto usage = [argv]() {
        std::cerr << "Usage: " << argv[0] << " [--test] path/to/File(.java|.class)..." << "\n";
        return -1;
    };

    if (argc < 2)
        return usage();

    bool is_test = argc == 3 && strcmp(argv[1], "--test") == 0;

    std::vector<ClassFile> class_files;
    FileDeleter file_deleter;

    for (int file_index = is_test ? 2 : 1; file_index < argc; ++file_index) {
        std::string path{argv[file_index]};

        if (path.ends_with(".java")) {
            std::stringstream ss{};
            ss << "javac " << path;
            auto command = ss.str();
            if (system(command.c_str()) != 0) {
                std::cerr << command << " failed";
                return -2;
            }

            path.erase(path.end() - strlen(".java"), path.end());
            path += ".class";
            file_deleter.add(path);
        } else if (path.ends_with(".class")) {
            // Nothing to do
        } else {
            return usage();
        }

        std::ifstream in{path, std::ios::in | std::ios::binary};
        if (in) {
            Parser parser{in};
            class_files.push_back(parser.parse());
        } else {
            std::cerr << "Failed to read file\n";
            return -2;
        }
    }

    if (is_test) {
        std::cout << "Hello, Test!" << std::endl;
    } else {
        std::cout << "Hello, World!" << std::endl;
    }
    return 0;
}
