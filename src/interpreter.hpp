#ifndef SCHOKOVM_INTERPRETER_HPP
#define SCHOKOVM_INTERPRETER_HPP

#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

#include "future.hpp"
#include "classfile.hpp"
#include "memory.hpp"

struct Stack;

/** https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.6 */
struct Frame {
    ClassFile *clazz;
    method_info *method;
    std::vector<u1> *code;

    // stack_memory indices (we cannot store pointers because the memory could move)
    std::span<Value> locals;
    std::span<Value> operands;
    size_t first_operand_index;
    // >= number of operands
    size_t operands_top;
    size_t previous_stack_memory_usage;

    // The index of the instruction that is currently being execute (= the invoke* instruction in parent frames).
    size_t pc;
    // The length of the invoke* instruction, to calculate the next instruction upon return
    // (only used when a parent frame is popped).
    unsigned char invoke_length;

    Frame(Stack &stack, ClassFile *clazz, method_info *method, size_t operand_stack_top);

    inline u1 read_u1() {
        return (*code)[pc + 1];
    }

    inline u1 consume_u1() {
        return (*code)[++pc];
    }

    inline u2 read_u2() {
        return static_cast<u2>(((*code)[pc + 1] << 8) | (*code)[pc + 2]);
    }

    inline u2 consume_u2() {
        u2 value = static_cast<u2>(((*code)[pc + 1] << 8) | (*code)[pc + 2]);
        pc += 2;
        return value;
    }

    inline Value peek_at(size_t offset) {
        return operands[operands_top - 1 - offset];
    }

    inline Value pop() {
        return operands[--operands_top];
    }

    inline Value pop2() {
        operands_top -= 2;
        return operands[operands_top];
    }

    inline void push(Value operand) {
        operands[operands_top++] = operand;
    }

    inline void push2(Value operand) {
        operands[operands_top] = operand;
        operands_top += 2;
    }

    inline void clear() {
        operands_top = 0;
    }

    template<typename Element>
    Element pop();

    template<typename Element>
    void push(Element value);
};

// category 1

template<>
inline void Frame::push<s4>(s4 value) {
    push(Value(value));
}

template<>
inline void Frame::push<bool>(bool value) {
    push<s4>(value);
}

template<>
inline void Frame::push<s1>(s1 value) {
    push<s4>(value);
}

template<>
inline void Frame::push<u2>(u2 value) {
    push<s4>(value);
}

template<>
inline void Frame::push<float>(float value) {
    push(Value(value));
}

template<>
inline void Frame::push<Reference>(Reference value) {
    push(Value(value));
}

template<>
inline s4 Frame::pop<s4>() {
    return pop().s4;
}

template<>
inline bool Frame::pop<bool>() {
    return static_cast<bool>(pop<s4>());
}

template<>
inline s1 Frame::pop<s1>() {
    return static_cast<s1>(pop<s4>());
}

template<>
inline u2 Frame::pop<u2>() {
    return static_cast<u2>(pop<s4>());
}

template<>
inline float Frame::pop<float>() {
    return pop().float_;
}

// category 2

template<>
inline Reference Frame::pop<Reference>() {
    return pop().reference;
}

template<>
inline void Frame::push<s8>(s8 value) {
    push2(Value(value));
}

template<>
inline void Frame::push<double>(double value) {
    push2(Value(value));
}

template<>
inline s8 Frame::pop<s8>() {
    return pop2().s8;
}

template<>
inline double Frame::pop<double>() {
    return pop2().double_;
}

//struct MethodArea {
//    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.5.4
//    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.1
//    //  a map (class) -> (runtime constant pool, data...)
//};

/** https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.5.1 */
struct Stack {
    // Local variables and the operand stack are stored like this:
    //
    //   |caller local variables| caller operand stack |
    //   |  n     n+1     n+2   | n+3 | n+4  n+5   n+6 |  n+7  n+8  |  n+9    n+10  | n+11
    //                                |   callee local variables    |callee operands|
    //  Timeline:
    //    1. The caller pushes arguments n+4,n+5,n+6               memory_used = n+7
    //    2. Invoke: top of parent operand stack becomes locals    memory_used = n+11
    //    3. Run the callee function
    //    4. Copy the return value from n+9 to n+4                memory_used = n+7
    //
    //  Dummy values:
    //    According to the jvm specification values of category 2 (long/double) always
    //    take up two local variable slots, and untyped stack instructions such as
    //    pop2 and dup2 rely on this assumption.
    //
    //    References only take up one slot though, which is not ideal on a 64 bit
    //    architecture. For this reason we decided to represent each value as a
    //    64 bit union. This also comes with the benefit, that we do not have to
    //    split up category 2 values. However, it also means that we use more memory
    //    per slot and that we have to keep empty slots with dummy values after
    //    values of category 2.
    //
    //    When we tried to eliminate the empty slots on the operand stack we ran
    //    into a problem: Some untyped stack instructions would have to know the
    //    category of the value on the stack. For example pop2 either removes two
    //    category 1 values or a single category 2 value. Another issue was, that
    //    since we didn't want to recompute indices of the local variables we had
    //    to use empty slots there, which made function calls more complicated,
    //    because we had to move/copy the function arguments that came after a
    //    category 2 value.
    //
    //    TLDR: All Values are 64 bit. The slot after longs and dobles is unused.
    //
    // WARNING: This must never be resized because we create spans of the elements!
    std::vector<Value> memory;
    size_t memory_used = 0;

    // In the interpreter we will keep the current frame in a local variable.
    std::vector<Frame> parent_frames;

    /// Pushes current_frame onto the list of parent frames and sets to current frame to run `method` in `clazz`
    void push_frame(Frame &current_frame, ClassFile *clazz, method_info *method) {
        size_t operand_stack_top = current_frame.first_operand_index + current_frame.operands_top;
        current_frame.operands_top += -method->stack_slots_for_parameters + method->return_category;
        parent_frames.push_back(current_frame);

        current_frame = {*this, clazz, method, operand_stack_top};
        if (memory_used > memory.size()) {
            throw std::runtime_error("stack overflow");
        }
    }
};

struct Thread {
    Stack stack{};
};

struct BootstrapClassLoader;

[[nodiscard]] bool
method_selection(ClassFile *dynamic_class, ClassFile *declared_class, method_info *declared_method,
                 ClassFile *&out_class, method_info *&out_method);


Value interpret(Thread &thread, ClassFile *main, method_info *method);

#endif //SCHOKOVM_INTERPRETER_HPP
