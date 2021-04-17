#ifndef SCHOKOVM_INTERPRETER_HPP
#define SCHOKOVM_INTERPRETER_HPP

#include <vector>
#include <memory>
#include <stack>
#include "classfile.hpp"

struct JVMLocalValue {
    JVMLocalValue() : u4_value(0) {}

    explicit JVMLocalValue(::u4 v) : u4_value(v) {}

    explicit JVMLocalValue(::s4 v) : s4_value(v) {}

    [[nodiscard]] inline float &float_() noexcept { return this->float_value; }

    [[nodiscard]] inline const float &float_() const noexcept { return this->float_value; }

    [[nodiscard]] inline ::u4 &u4() noexcept { return u4_value; }

    [[nodiscard]] inline const ::u4 &u4() const noexcept { return u4_value; }

    [[nodiscard]] inline ::s4 &s4() noexcept { return s4_value; }

    [[nodiscard]] inline const ::s4 &s4() const noexcept { return s4_value; }

//    // actual type instead of void
//    [[nodiscard]] inline const void* &reference() const noexcept { return value.s4; }

private:
    union {
        ::u4 u4_value;
        ::s4 s4_value;
        float float_value;
    };
};

struct JVMStackValue {
    explicit JVMStackValue(float v) : value({.float_ = v}) {}

    explicit JVMStackValue(::u4 v) : value({.u4 = v}) {}

    explicit JVMStackValue(::s4 v) : value({.s4 = v}) {}

    explicit JVMStackValue(::u8 v) : value({.u8 = v}) {}

    explicit JVMStackValue(::s8 v) : value({.s8 = v}) {}

    [[nodiscard]] inline const float &float_() const noexcept { return value.float_; }

    [[nodiscard]] inline const ::u4 &u4() const noexcept { return value.u4; }

    [[nodiscard]] inline const ::s4 &s4() const noexcept { return value.s4; }

    [[nodiscard]] inline const ::u8 &u8() const noexcept { return value.u8; }

    [[nodiscard]] inline const ::s8 &s8() const noexcept { return value.s8; }

private:
    union {
        ::u4 u4;
        ::s4 s4;
        ::u8 u8;
        ::s8 s8;
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
