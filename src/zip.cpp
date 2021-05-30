#include <optional>
#include <vector>
#include <zip.h>

#include "classfile.hpp"
#include "zip.hpp"
#include "parser.hpp"

ZipException::ZipException(std::string message) : message(std::move(message)) {}

const char *ZipException::what() const noexcept {
    return message.c_str();
}

// https://stackoverflow.com/a/7782037
struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

void read_entire_jar(const char *path, std::vector<char> &buffer, std::vector<ClassFile> &class_files) {
    ZipArchive zip{path};

    for (const auto &item : zip.entries) {
        auto const &name = item.first;
        auto const &entry = item.second;

        if (!name.ends_with(".class")) {
            continue;
        }

        if (buffer.size() < entry.size) {
            buffer.resize(std::max((size_t) entry.size, buffer.size() * 2));
        }

        zip_file_t *file = zip_fopen_index(zip.archive.get(), entry.index, 0);
        if (file == nullptr) {
            throw ZipException("zip_fopen_index");
        }

        zip_int64_t length = zip_fread(file, buffer.data(), buffer.size());

        if (zip_fclose(file) != 0) {
            throw ZipException("zip_fclose failed (zip_fread length was " + std::to_string(length) + ")");
        }

        if (length == -1) {
            throw ZipException("zip_fread");
        }

        // TODO maybe the parser should simply use a buffer instead of istream?
        membuf buf{buffer.data(), buffer.data() + length};
        std::istream in{&buf};
        Parser parser{in};
        class_files.push_back(parser.parse());
    }
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
