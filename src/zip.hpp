#ifndef SCHOKOVM_ZIP_HPP
#define SCHOKOVM_ZIP_HPP

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
    size_t size;
};

struct ZipArchive {
    zip_t *archive;
    std::string path;
    std::unordered_map<std::string, ZipEntry> entries;

    explicit ZipArchive(std::string path);

    ~ZipArchive();

    void close();
};

#endif //SCHOKOVM_ZIP_HPP
