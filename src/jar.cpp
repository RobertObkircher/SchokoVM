#include <optional>
#include <vector>
#include <zip.h>

#include "classfile.hpp"
#include "jar.hpp"
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
    if (buffer.size() < zip.max_size) {
        buffer.resize(zip.max_size);
    }

    for (const auto &item : zip.entries) {
        auto const &name = item.first;
        auto const &entry = item.second;

        if (!name.ends_with(".class")) {
            continue;
        }
        assert(entry.stat_size <= zip.max_size);

        zip_file_t *file = zip_fopen_index(zip.archive, entry.index, 0);
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
    zip.close();
}

ZipArchive::ZipArchive(std::string path) : path(std::move(path)) {
    int error = 0;
    archive = zip_open(this->path.c_str(), ZIP_RDONLY, &error);
    if (!archive) {
        throw ZipException("zip_open: " + std::to_string(error));
    }

    zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t index = 0; index < (zip_uint64_t) num_entries; index++) {
        zip_stat_t stat;
        zip_stat_init(&stat);
        zip_stat_index(archive, index, 0, &stat);

        if ((stat.valid & ZIP_STAT_NAME) == 0) {
            throw ZipException("zip_stat name");
        }

        if ((stat.valid & ZIP_STAT_SIZE) == 0) {
            throw ZipException("zip_stat size");
        }

        entries.insert({{stat.name},
                        {index, stat.size}});

        max_size = std::max(max_size, stat.size);
    }
}

void ZipArchive::close() {
    if (archive == nullptr) {
        return;
    }

    int result = zip_close(archive);
    archive = nullptr;
    if (result != 0) {
        throw ZipException("zip_close: " + std::to_string(result));
    }
}

ZipArchive::~ZipArchive() {
    if (archive != nullptr) {
        int result = zip_close(archive);
        // if this fails there is nothing that we can do
        assert(result == 0);
        (void) result; // supress unused variable warning in release builds
    }
}
