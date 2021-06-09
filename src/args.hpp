#ifndef SCHOKOVM_ARGS_HPP
#define SCHOKOVM_ARGS_HPP

#include <optional>
#include <string>
#include <vector>

struct Arguments {
    std::string mainclass;
    std::string classpath;
    std::string libjava;
    std::vector<std::string> remaining;
};

std::optional<Arguments> parse_args(int argc, char *argv[]);

#endif //SCHOKOVM_ARGS_HPP
