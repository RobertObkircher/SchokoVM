#ifndef SCHOKOVM_INTERPRETER_HPP
#define SCHOKOVM_INTERPRETER_HPP

#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

#include "future.hpp"
#include "classfile.hpp"

union Value {
    Value() : s8(0) {}

    // NOLINTNEXTLINE
    Value(::u4 v) : s4(future::bit_cast<::s4>(v)) {}

    // NOLINTNEXTLINE
    Value(::s4 v) : s4(v) {}

    // NOLINTNEXTLINE
    Value(::u8 v) : s8(future::bit_cast<::s8>(v)) {}

    // NOLINTNEXTLINE
    Value(::s8 v) : s8(v) {}

    // NOLINTNEXTLINE
    Value(float v) : float_(v) {}

    // NOLINTNEXTLINE
    Value(double v) : double_(v) {}

    ::s4 s4;
    ::s8 s8;
    float float_;
    double double_;
};

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
    size_t operands_count;
    size_t previous_stack_memory_usage;

    size_t pc;

    Frame(Stack &stack, ClassFile *clazz, method_info *method, size_t operand_stack_top);

    Value stack_pop() {
        return operands[--operands_count];
    }

    void stack_push(float v) {
        operands[operands_count++] = Value(v);
    }

    void stack_push(double v) {
        operands[operands_count++] = Value(v);
    }

    void stack_push(s4 v) {
        operands[operands_count++] = Value(v);
    }

    void stack_push(u4 v) {
        operands[operands_count++] = Value(v);
    }

    void stack_push(u8 v) {
        operands[operands_count++] = Value(v);
    }

    void stack_push(s8 v) {
        operands[operands_count++] = Value(v);
    }

    void stack_push(Value v) {
        operands[operands_count++] = v;
    }
};

//struct MethodArea {
//    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.5.4
//    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.1
//    //  a map (class) -> (runtime constant pool, data...)
//};

//struct Heap {
//  would be needed for GC?
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
    //    3. Insert empty slots if there are longs/doubles
    //    4. Run the callee function
    //    3. Copy the return value from n+9 to n+4                memory_used = n+7
    //
    // WARNING: This must never be resized because we create spans of the elements!
    std::vector<Value> memory;
    size_t memory_used = 0;

    // In the interpreter we will keep the current frame in a local variable.
    std::vector<Frame> parent_frames;
};

struct Thread {
    Stack stack{};
};

int interpret(std::unordered_map<std::string_view, ClassFile *> &class_files, ClassFile *main);

#endif //SCHOKOVM_INTERPRETER_HPP
