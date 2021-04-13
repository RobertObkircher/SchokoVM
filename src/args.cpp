#include "args.hpp"

#include <iostream>

std::optional<Arguments> parse_args(int argc, char *argv[]) {
    auto usage = [argv](const std::string &error_message) {
        if (!error_message.empty()) {
            std::cerr << "Error: " << error_message << "\n";
        }
        std::cerr << "Usage: " << argv[0] << " [options] <mainclass> [args...]\n"
                  << "  where the options are:\n"
                  << "    --test <java> <outdir>      Compare against a reference java implementation. Files are stored in outdir.\n"
                  << "    --class-path <classpath>    Where to search for classes. Default is the current directory.\n";
        return std::optional<Arguments>{};
    };

    std::optional<std::string> classpath{};
    std::optional<std::string> mainclass{};
    std::vector<std::string> remaining;

    int index = 1;
    while (index < argc) {
        std::string arg{argv[index]};
        ++index;

        if (arg == "--") {
            break;
        } else if (arg == "--class-path" || arg == "-cp" || arg == "-classpath") {
            if (classpath) {
                return usage("Classpath is specified twice");
            } else if (index >= argc) {
                return usage("Expected argument after" + arg);
            } else {
                classpath = argv[index++];
            }
        } else {
            mainclass = arg;
            break;
        }
    }
    while (index < argc) {
        remaining.emplace_back(argv[index++]);
    }

    if (!mainclass)
        return usage("mainclass must be specified!");

    if (!classpath)
        classpath = ".";

    return Arguments{
            *mainclass,
            *classpath,
            remaining,
    };
}
