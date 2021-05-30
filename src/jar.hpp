#ifndef SCHOKOVM_JAR_HPP
#define SCHOKOVM_JAR_HPP

#include <unordered_map>
#include <zip.h>

void read_entire_jar(const char *path, std::vector<char> &buffer, std::vector<ClassFile> &class_files);

struct ZipException : std::exception {
    std::string message;

    explicit ZipException(std::string message);

    [[nodiscard]] const char *what() const noexcept override;
};

struct ZipEntry {
    zip_uint64_t index;
    size_t stat_size;
};

struct ZipArchive {
    zip_t *archive;
    std::string path;
    std::unordered_map<std::string, ZipEntry> entries;
    size_t max_size;

    explicit ZipArchive(std::string path);

    ~ZipArchive();

    void close();
};

#endif //SCHOKOVM_JAR_HPP
