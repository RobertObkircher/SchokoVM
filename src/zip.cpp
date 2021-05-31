#include <optional>
#include <vector>
#include <zip.h>

#include "zip.hpp"

ZipException::ZipException(std::string message) : message(std::move(message)) {}

const char *ZipException::what() const noexcept {
    return message.c_str();
}

ZipArchive::ZipArchive() : archive(), path(), entries() {
}

ZipArchive::ZipArchive(std::string path) : archive(), path(std::move(path)), entries() {
    int error = 0;
    archive.reset(zip_open(this->path.c_str(), ZIP_RDONLY, &error));
    if (archive == nullptr) {
        throw ZipException("zip_open: " + std::to_string(error));
    }

    zip_int64_t num_entries = zip_get_num_entries(archive.get(), 0);

    for (zip_uint64_t index = 0; index < (zip_uint64_t) num_entries; index++) {
        zip_stat_t stat;
        zip_stat_init(&stat);
        zip_stat_index(archive.get(), index, 0, &stat);

        if ((stat.valid & ZIP_STAT_NAME) == 0) {
            throw ZipException("zip_stat name");
        }

        if ((stat.valid & ZIP_STAT_SIZE) == 0) {
            throw ZipException("zip_stat size");
        }

        entries.insert({{stat.name}, {index, stat.size}});
    }
}

ZipEntry const *ZipArchive::entry_for_path(std::string &filepath) const {
    if (auto found = entries.find(filepath); found != entries.end()) {
        return &found->second;
    } else {
        return nullptr;
    }
}

void ZipArchive::read(ZipEntry const &entry, std::vector<char> &buffer) const {
    buffer.resize(entry.size);

    zip_file_t *file = zip_fopen_index(archive.get(), entry.index, 0);
    if (file == nullptr) {
        throw ZipException("zip_fopen_index");
    }

    zip_int64_t length = zip_fread(file, buffer.data(), buffer.size());

    if (zip_fclose(file) != 0) {
        throw ZipException("zip_fclose failed (zip_fread length was " + std::to_string(length) + ")");
    }

    if (length == -1) {
        throw ZipException("zip_fread failed");
    }

    if (static_cast<size_t>(length) != entry.size) {
        throw ZipException("zip_fread: length was " + std::to_string(length) + " but the expected size was " +
                           std::to_string(entry.size));
    }
}

