#ifndef SCHOKOVM_UTIL_HPP
#define SCHOKOVM_UTIL_HPP

#include <vector>
#include <string>

std::vector<std::string> split(std::string const &string, char separator);

int get_signal_number(const char *signal_name);

#endif //SCHOKOVM_UTIL_HPP
