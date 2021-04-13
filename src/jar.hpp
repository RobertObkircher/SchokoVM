#ifndef SCHOKOVM_JAR_HPP
#define SCHOKOVM_JAR_HPP

std::optional<std::string>
read_entire_jar(const char *path, std::vector<char> &buffer, std::vector<ClassFile> &class_files);

#endif //SCHOKOVM_JAR_HPP
