#ifndef SCHOKOVM_INTERPRETER_HPP
#define SCHOKOVM_INTERPRETER_HPP

#include <vector>
#include <memory>
#include <stack>
#include "classfile.hpp"

union Value {
    Value() : u8(0) {}

    explicit Value(::u4 v) : u4(v) {}

    explicit Value(::s4 v) : s4(v) {}

    explicit Value(::u8 v) : u8(v) {}

    explicit Value(::s8 v) : s8(v) {}

    explicit Value(float v) : float_(v) {}

    explicit Value(double v) : double_(v) {}

    ::u4 u4;
    ::s4 s4;
    ::u8 u8;
    ::s8 s8;
    float float_;
    double double_;
};

/** https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.6 */
struct Frame {
    const ClassFile &clas;

    std::vector<Value> locals;
    std::vector<Value> stack;

    // TODO the runtime access pool?
    // void *constant_pool;

    /** the calling/previous frame */
    std::unique_ptr<Frame> previous_frame;

    Frame(const ClassFile &clas, size_t locals_count, size_t stack_count, std::unique_ptr<Frame> previous_frame)
            : clas(clas), previous_frame(std::move(previous_frame)) {
        this->locals.resize(locals_count);
        this->stack.reserve(stack_count);
    }

    Value stack_pop() {
        auto v = this->stack.back();
        this->stack.pop_back();
        return v;
    }

    void stack_push(float v) {
        this->stack.emplace_back(Value(v));
    }

    void stack_push(s4 v) {
        this->stack.emplace_back(Value(v));
    }

    void stack_push(u4 v) {
        this->stack.emplace_back(Value(v));
    }

    void stack_push(u8 v) {
        this->stack.emplace_back(Value(v));
    }

    void stack_push(s8 v) {
        this->stack.emplace_back(Value(v));
    }
};

/**
 * TODO ???
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

int interpret(const std::vector<ClassFile> &class_files, size_t main_class_index);

#endif //SCHOKOVM_INTERPRETER_HPP
