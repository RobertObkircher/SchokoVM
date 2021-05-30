#include <vector>
#include <string>

#include "util.hpp"

std::vector<std::string> split(std::string const &string, char separator) {
    std::vector<std::string> parts{};
    size_t start = 0;
    size_t end;
    while ((end = string.find(separator, start)) != std::string::npos) {
        parts.push_back(string.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(string.substr(start));
    return parts;
}
