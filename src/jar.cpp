#include <optional>
#include <vector>
#include <zip.h>

#include "classfile.hpp"
#include "jar.hpp"
#include "parser.hpp"

// https://stackoverflow.com/a/7782037
struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

struct ZipCloser {
    zip_t *archive;

    explicit ZipCloser(zip_t *archive) : archive(archive) {}

    [[nodiscard]] inline int close() {
        if (archive != nullptr) {
            int result = zip_close(archive);
            archive = nullptr;
            return result;
        } else {
            return 0;
        }
    }

    // TODO logging?
    // there is nothing that we can do other than silently ignoring the error
    virtual ~ZipCloser() {
        int error = close();
        (void) error;
    }
};

std::optional<std::string>
read_entire_jar(const char *path, std::vector<char> &buffer, std::vector<ClassFile> &class_files) {
    int error = 0;
    zip_t *archive = zip_open(path, ZIP_RDONLY, &error);
    if (!archive) return "zip_open";
    ZipCloser zip_closer{archive};
    zip_int64_t num_entries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t index = 0; index < (zip_uint64_t) num_entries; index++) {
        zip_stat_t stat;
        zip_stat_init(&stat);
        zip_stat_index(archive, index, 0, &stat);

        if (!(stat.valid & ZIP_STAT_NAME)) return "zip_stat name";

        std::string name{stat.name};
        if (!name.ends_with(".class")) continue;

        if (!(stat.valid & ZIP_STAT_SIZE)) return "zip_stat size";

        if (buffer.size() < stat.size) buffer.resize(std::max((size_t) stat.size, buffer.size() * 2));

        zip_file_t *file = zip_fopen_index(archive, index, 0);
        if (!file) return "zip_fopen";
        zip_int64_t length = zip_fread(file, buffer.data(), buffer.size());
        if (zip_fclose(file) != 0) return "zip_fclose";
        if (length == -1) return "zip_fread"; // after closing

        // TODO maybe the parser should simply use a buffer instead of istream?
        membuf buf{buffer.data(), buffer.data() + length};
        std::istream in{&buf};
        Parser parser{in};
        class_files.push_back(parser.parse());
    }
    if (zip_closer.close() != 0) return "zip_close";
    return {};
}
