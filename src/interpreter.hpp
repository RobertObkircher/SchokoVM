#ifndef SCHOKOVM_INTERPRETER_HPP
#define SCHOKOVM_INTERPRETER_HPP

#include <vector>
#include <memory>
#include <stack>
#include "classfile.hpp"

union JVMLocalValue {
    JVMLocalValue() : u4_(0) {}

    explicit JVMLocalValue(u4 v) : u4_(v) {}

    explicit JVMLocalValue(s4 v) : s4_(v) {}

    u4 u4_;
    s4 s4_;
    float float_;
};

struct JVMStackValue {
    explicit JVMStackValue(float v) : value({.float_ = v}) {}

    explicit JVMStackValue(u4 v) : value({.u4_ = v}) {}

    explicit JVMStackValue(s4 v) : value({.s4_ = v}) {}

    explicit JVMStackValue(u8 v) : value({.u8_ = v}) {}

    explicit JVMStackValue(s8 v) : value({.s8_ = v}) {}

    [[nodiscard]] inline const float &float_() const noexcept { return value.float_; }

    [[nodiscard]] inline const u4 &u4_() const noexcept { return value.u4_; }

    [[nodiscard]] inline const s4 &s4_() const noexcept { return value.s4_; }

    [[nodiscard]] inline const u8 &u8_() const noexcept { return value.u8_; }

    [[nodiscard]] inline const s8 &s8_() const noexcept { return value.s8_; }

private:
    union {
        u4 u4_;
        s4 s4_;
        u8 u8_;
        s8 s8_;
        float float_;
//        double double_;
    } value;
};

/** https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.6 */
struct Frame {
    const ClassFile &clas;

    std::vector<JVMLocalValue> locals;
    std::vector<JVMStackValue> stack;

    // TODO the runtime access pool?
    // void *constant_pool;

    /** the calling/previous frame */
    std::unique_ptr<Frame> previous_frame;

    Frame(const ClassFile &clas, size_t locals_count, std::unique_ptr<Frame> previous_frame)
            : clas(clas), previous_frame(std::move(previous_frame)) {
        this->locals.resize(locals_count);
    }

    JVMStackValue stack_pop() {
        auto v = this->stack.back();
        this->stack.pop_back();
        return v;
    }

    void stack_push(float v) {
        this->stack.emplace_back(JVMStackValue(v));
    }

    void stack_push(s4 v) {
        this->stack.emplace_back(JVMStackValue(v));
    }

    void stack_push(u4 v) {
        this->stack.emplace_back(JVMStackValue(v));
    }

    void stack_push(u8 v) {
        this->stack.emplace_back(JVMStackValue(v));
    }

    void stack_push(s8 v) {
        this->stack.emplace_back(JVMStackValue(v));
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
