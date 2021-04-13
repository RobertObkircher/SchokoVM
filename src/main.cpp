#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

#include "args.hpp"
#include "classfile.hpp"
#include "jar.hpp"

const char *compare_script = R"COMPARE_SCRIPT(
JAVA="$1"
CLASSPATH="$2"
CLASS="$3"
OUT="$4"

R_OUT="$OUT/reference_out.txt"
R_ERR="$OUT/reference_err.txt"
R_STATUS="$OUT/reference_status.txt"

S_OUT="$OUT/schoko_out.txt"
S_ERR="$OUT/schoko_err.txt"
S_STATUS="$OUT/schoko_status.txt"

"$JAVA" --class-path "$CLASSPATH" "$CLASS" 1> "$R_OUT" 2> "$R_ERR"
echo "$?" > "$R_STATUS"

X=0
echo "============================== status =============================="
diff --side-by-side "$R_STATUS" "$S_STATUS" || X=1
echo "============================== stdout =============================="
diff --side-by-side "$R_OUT" "$S_OUT" || X=1
echo "============================== stderr =============================="
diff --side-by-side "$R_ERR" "$S_ERR" || X=1
echo "===================================================================="
exit "$X"
)COMPARE_SCRIPT";

int run(const Arguments &arguments) {
    std::vector<ClassFile> class_files;

    auto classpath = arguments.classpath;

    // TODO split by : and read normal directories
    if ((classpath.find(':') != std::string::npos || classpath.find(".jar") != classpath.length() - 4)) {
        std::cerr << "unimplemented: the classpath must consist of a single jar file";
        return -6;
    }

    {
        std::vector<char> buffer;
        buffer.resize(1024);
        const char *path = classpath.c_str();
        auto error_message = read_entire_jar(path, buffer, class_files);
        if (error_message) {
            std::cerr << "Could not read jar file " << path << " error: " << *error_message << "\n";
            return 23;
        }
    }

//    std::ifstream in{classpath, std::ios::in | std::ios::binary};
//    if (in) {
//        Parser parser{in};
//        class_files.push_back(parser.parse());
//    } else {
//        std::cerr << "Failed to read file\n";
//        return -2;
//    }

    ssize_t index = -1;
    for (size_t i = 0; i < class_files.size(); ++i) {
        auto &item = class_files[i];
        auto &name = item.constant_pool.utf8_strings[item.constant_pool.table[item.constant_pool.table[item.this_class].info.string_info.string_index].info.utf_8_info.index];
        if (name == arguments.mainclass) {
            index = static_cast<ssize_t>(i);
        }
    }
    if (index == -1) {
        std::cerr << "mainclass was not found!\n";
        return 5;
    }

    // TODO actually execute java code...
    std::cout << "Hello, world!\n";
    return 0;
}

class Redirect {
    std::ostream &it;
    std::streambuf *buf;
public:
    Redirect(std::ostream &it, std::ostream &where) : it(it) {
        buf = it.rdbuf();
        it.rdbuf(where.rdbuf());
    }

    virtual ~Redirect() {
        it.flush();
        it.rdbuf(buf);
    }
};

int main(int argc, char *argv[]) {
    std::optional<Arguments> arguments = parse_args(argc, argv);
    if (!arguments) return 23;

    if (!arguments->test) {
        return run(*arguments);
    } else {
        // TODO I'd prefer If we didn't have to do it like this
        // The problem is that if a script launches a new process gdb doesn't follow it (by default).
        // Maybe the script could launch something like gdb --connect_to_clion ./SchokoVM?
        // ~/.gdbinit
        //   set auto-load local-gdbinit on
        //   add-auto-load-safe-path /
        // SchokoVM/.gdbinit
        //   set follow-fork-mode child   # would debug mkdirs since that is executed first
        //   set detach-on-fork off       # didn't seem to work

        std::string output{arguments->test->second};
        std::filesystem::create_directories(output);

        {
            std::ofstream cout_file{output + "/schoko_out.txt"};
            std::ofstream cerr_file{output + "/schoko_err.txt"};
            std::ofstream status_file{output + "/schoko_status.txt"};
            Redirect out{std::cout, cout_file};
            Redirect err{std::cerr, cerr_file};
            int status = run(*arguments);
            status_file << status << "\n";
        }

        const char *shell = "/usr/bin/sh";
        const char *script_argv[] = {
                shell,
                "-c",
                compare_script,
                "--",
                arguments->test->first.c_str(),
                arguments->classpath.c_str(),
                arguments->mainclass.c_str(),
                arguments->test->second.c_str(),
                nullptr,
        };
        execv(shell, (char **) script_argv);
        std::cout << "Could not exectue compare script\n";
        return 42;
    }
}