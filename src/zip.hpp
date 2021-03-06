#ifndef SCHOKOVM_ZIP_HPP
#define SCHOKOVM_ZIP_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <zip.h>

struct ZipException : std::exception {
    std::string message;

    explicit ZipException(std::string message);

    [[nodiscard]] const char *what() const noexcept override;
};

struct ZipEntry {
    zip_uint64_t index;
    size_t size;
};

struct ZipDeleter {
    void operator()(zip_t *pointer) noexcept {
        zip_discard(pointer);
    }
};

struct ZipArchive {
    std::unique_ptr<zip_t, ZipDeleter> archive;
    std::string path;
    std::unordered_map<std::string, ZipEntry> entries;

    ZipArchive();

    explicit ZipArchive(std::string path);

    ZipEntry const *entry_for_path(std::string const &filepath) const;

    void read(ZipEntry const &entry, std::vector<char> &buffer) const;
};

#endif //SCHOKOVM_ZIP_HPP
