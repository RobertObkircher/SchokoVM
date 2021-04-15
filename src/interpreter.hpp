#ifndef SCHOKOVM_INTERPRETER_HPP
#define SCHOKOVM_INTERPRETER_HPP

#include <vector>
#include <stack>
#include "classfile.hpp"

/** https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.6 */
struct Frame {
    const ClassFile &clas;

    std::vector<u4> locals;
    std::stack<u8> stack;

    // TODO???
    // void *constant_pool;

    /** the calling/previous frame */
    std::unique_ptr<Frame> previous_frame;

    Frame(const ClassFile &clas, size_t locals_count, std::unique_ptr<Frame> previous_frame);
};

/**
 * https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.5.4
 * https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.1
 */
struct MethodArea {
    // a map (class) -> (runtime constant pool, data...)
};

//struct Heap {
//  would be needed for GC?
//};

/** https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.5.1 */
struct Stack {
    std::unique_ptr<Frame> current_frame;
};

int interpret(const std::vector<ClassFile> &class_files, ssize_t main_class_index);

#endif //SCHOKOVM_INTERPRETER_HPP
