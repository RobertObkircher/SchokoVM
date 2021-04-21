#include "interpreter.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <iostream>
#include <type_traits>
#include "opcodes.hpp"
#include "future.hpp"

static const u2 MAIN_ACCESS_FLAGS = (static_cast<u2>(FieldInfoAccessFlags::ACC_PUBLIC) |
                                     static_cast<u2>(FieldInfoAccessFlags::ACC_STATIC));
static const auto MAIN_NAME = "main";
static const auto MAIN_DESCRIPTOR = "([Ljava/lang/String;)V";

static size_t execute_instruction(Frame &frame, const std::vector<u1> &code, size_t pc, bool &shouldExit);

static size_t execute_comparison(const std::vector<u1> &code, size_t pc, bool condition);

static size_t goto_(const std::vector<u1> &code, size_t pc);

template<typename T>
static inline T add_overflow(T a, T b) {
    // C++20 requires 2's complement for signed integers
    return future::bit_cast<T>(
            future::bit_cast<std::make_unsigned_t<T>>(a) + future::bit_cast<std::make_unsigned_t<T>>(b)
    );
}

template<typename T>
static inline T sub_overflow(T a, T b) {
    // C++20 requires 2's complement for signed integers
    if (b == std::numeric_limits<T>::min()) {
        // the most negative number would overflow when changing the sign
        return future::bit_cast<T>(
                // a - (b + 1) = a - b - 1
                future::bit_cast<std::make_unsigned_t<T>>(a) +
                future::bit_cast<std::make_unsigned_t<T>>(-(b + 1)) +
                1
        );
    } else {
        return future::bit_cast<T>(
                future::bit_cast<std::make_unsigned_t<T>>(a) - future::bit_cast<std::make_unsigned_t<T>>(b)
        );
    }
}

template<typename T>
static inline T mul_overflow(T a, T b) {
    // C++20 requires 2's complement for signed integers
    auto a_u = static_cast<std::make_unsigned_t<T>>(a);
    auto b_u = static_cast<std::make_unsigned_t<T>>(b);
    auto result = static_cast<T>(a_u * b_u);
    return result;
}

template<typename T>
static inline T div_overflow(T dividend, T divisor) {
    if (dividend == std::numeric_limits<T>::min() && divisor == -1) {
        return dividend;
    }
    // TODO test rounding
    return dividend / divisor;
}

int interpret(const std::vector<ClassFile> &class_files, size_t main_class_index) {
    // 1. Find the main method
    const ClassFile &main = class_files[main_class_index];
    auto main_method_iter = std::find_if(main.methods.begin(), main.methods.end(),
                                         [](const method_info &m) {
                                             return m.name_index->value == MAIN_NAME &&
                                                    m.descriptor_index->value == MAIN_DESCRIPTOR &&
                                                    (m.access_flags & MAIN_ACCESS_FLAGS) == MAIN_ACCESS_FLAGS;
                                         }
    );
    if (main_method_iter == std::end(main.methods)) {
        throw std::runtime_error("Couldn't find main method");
    }

    // 2. Actually start running the code
    const auto &main_method = *main_method_iter;
    // TODO find the code attribute properly
    const auto &code = std::get<Code_attribute>(main_method.attributes[0].variant);

    Stack stack;
    std::unique_ptr<Frame> p(new Frame(main, code.max_locals, code.max_stack, nullptr));
    stack.current_frame = std::move(p);

    size_t pc = 0;
    bool shouldExit = false;
    while (true) {
        pc = execute_instruction(*stack.current_frame, code.code, pc, shouldExit);
        if (shouldExit) {
            break;
        }
    }

    // print exit code
    if (stack.current_frame->stack.empty()) {
        return 0;
    } else {
        return stack.current_frame->stack_pop().s4;
    }
}

static size_t execute_instruction(Frame &frame, const std::vector<u1> &code, size_t pc, bool &shouldExit) {
    auto opcode = code[pc];
    // TODO implement remaining opcodes. The ones that are currently commented/missing out have no test coverage whatsoever
    switch (static_cast<OpCodes>(opcode)) {
        /* ======================= Constants ======================= */
        case OpCodes::nop:
            break;
//        case OpCodes::aconst_null:
//            // "nullptr"
//            frame.stack_push(0);
//            break;
        case OpCodes::iconst_m1:
        case OpCodes::iconst_0:
        case OpCodes::iconst_1:
        case OpCodes::iconst_2:
        case OpCodes::iconst_3:
        case OpCodes::iconst_4:
        case OpCodes::iconst_5:
            frame.stack_push(opcode - static_cast<u1>(OpCodes::iconst_0));
            break;

        case OpCodes::lconst_0:
        case OpCodes::lconst_1:
            frame.stack_push(static_cast<s8>(opcode - static_cast<u1>(OpCodes::lconst_0)));
            break;
        case OpCodes::fconst_0:
        case OpCodes::fconst_1:
        case OpCodes::fconst_2:
            frame.stack_push(static_cast<float>(opcode - static_cast<u1>(OpCodes::fconst_0)));
            break;
        case OpCodes::dconst_0:
        case OpCodes::dconst_1:
            frame.stack_push(static_cast<double>(opcode - static_cast<u1>(OpCodes::dconst_0)));
            break;
        case OpCodes::bipush: {
            s4 value = static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(code[pc + 1])));
            frame.stack_push(value);
            return pc + 2;
        }
        case OpCodes::sipush: {
            u2 value = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
            frame.stack_push(static_cast<s4>(future::bit_cast<s2>(value)));
            return pc + 3;
        }
// TODO the following instructions actually read from the runtime constant pool
        case OpCodes::ldc: {
            auto index = code[pc + 1];
            auto &entry = frame.clas.constant_pool.table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.stack_push(i->bytes);
//            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
//                frame.stack_push(f->value);
            } else {
                throw std::runtime_error("ldc refers to invalid/unimplemented type");
            }
            return pc + 2;
        }
//        case OpCodes::ldc_w:
        case OpCodes::ldc2_w: {
            size_t index = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
            auto &entry = frame.clas.constant_pool.table[index];
            if (auto l = std::get_if<CONSTANT_Long_info>(&entry.variant)) {
                frame.stack_push(l->value);
//            } else if (auto d = std::get_if<CONSTANT_Double_info>(&entry.variant)) {
//                frame.stack_push(d->value);
            } else {
                throw std::runtime_error("ldc2_w refers to invalid/unimplemented type");
            }
            return pc + 3;
        }

            /* ======================= Loads ======================= */
        case OpCodes::iload: {
            auto local = code[pc + 1];
            frame.stack_push(frame.locals[local].u4);
            return pc + 2;
        }
        case OpCodes::fload: {
            auto local = code[pc + 1];
            frame.stack_push(frame.locals[local].float_);
            return pc + 2;
        }
        case OpCodes::lload: {
            auto index = code[pc + 1];
            auto high = frame.locals[index].u4;
            auto low = frame.locals[index + 1].u4;
            u8 value = (static_cast<u8>(high) << 32) | static_cast<u8>(low);
            frame.stack_push(value);
            return pc + 2;
        }
//        case OpCodes::dload:
//        case OpCodes::aload:
        case OpCodes::iload_0:
        case OpCodes::iload_1:
        case OpCodes::iload_2:
        case OpCodes::iload_3: {
            auto value = frame.locals[opcode - static_cast<u1>(OpCodes::iload_0)].u4;
            frame.stack_push(value);
            break;
        }
        case OpCodes::lload_0:
        case OpCodes::lload_1:
        case OpCodes::lload_2:
        case OpCodes::lload_3: {
            u1 index = opcode - static_cast<u1>(OpCodes::lload_0);
            auto high = frame.locals[index].u4;
            auto low = frame.locals[index + 1].u4;
            frame.stack_push((static_cast<u8>(high) << 32) | low);
            break;
        }

            /* ======================= Stores ======================= */
        case OpCodes::istore:
            frame.locals[code[pc + 1]].u4 = frame.stack_pop().u4;
            return pc + 2;
        case OpCodes::lstore: {
            auto value = frame.stack_pop().u8;
            frame.locals[code[pc + 1]].u4 = (value >> 32) & U4_BIT_MASK;
            frame.locals[code[pc + 1] + 1].u4 = value & U4_BIT_MASK;
            return pc + 2;
        }
        case OpCodes::istore_0:
        case OpCodes::istore_1:
        case OpCodes::istore_2:
        case OpCodes::istore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::istore_0)].u4 = frame.stack_pop().u4;
            break;
        case OpCodes::lstore_0:
        case OpCodes::lstore_1:
        case OpCodes::lstore_2:
        case OpCodes::lstore_3: {
            auto value = frame.stack_pop().u8;
            frame.locals[opcode - static_cast<u1>(OpCodes::lstore_0)].u4 = (value >> 32) & U4_BIT_MASK;
            frame.locals[opcode - static_cast<u1>(OpCodes::lstore_0) + 1].u4 = value & U4_BIT_MASK;
            break;
        }

            /* ======================= Math =======================*/
        case OpCodes::iadd: {
            auto b = frame.stack_pop().s4;
            auto a = frame.stack_pop().s4;
            auto result = add_overflow(a, b);
            frame.stack_push(result);
            break;
        }
        case OpCodes::ladd: {
            auto b = frame.stack_pop().s8;
            auto a = frame.stack_pop().s8;
            auto result = add_overflow(a, b);
            frame.stack_push(result);
            break;
        }
        case OpCodes::isub: {
            auto b = frame.stack_pop().s4;
            auto a = frame.stack_pop().s4;
            auto result = sub_overflow(a, b);
            frame.stack_push(result);
            break;
        }
        case OpCodes::lsub: {
            s8 b = frame.stack_pop().s8;
            s8 a = frame.stack_pop().s8;
            frame.stack_push(sub_overflow(a, b));
            break;
        }
        case OpCodes::imul: {
            auto a = frame.stack_pop().s4;
            auto b = frame.stack_pop().s4;
            frame.stack_push(mul_overflow(a, b));
            break;
        }
        case OpCodes::lmul: {
            auto a = frame.stack_pop().s8;
            auto b = frame.stack_pop().s8;
            frame.stack_push(mul_overflow(a, b));
            break;
        }

        case OpCodes::iinc: {
            auto local = code[pc + 1];
            auto value = static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(code[pc + 2])));
            auto result = add_overflow(frame.locals[local].s4, value);
            frame.locals[local].s4 = result;
            return pc + 3;
        }

        case OpCodes::idiv: {
            auto divisor = frame.stack_pop().s4;
            auto dividend = frame.stack_pop().s4;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.stack_push(div_overflow(dividend, divisor));
            break;
        }
        case OpCodes::ldiv: {
            auto divisor = frame.stack_pop().s8;
            auto dividend = frame.stack_pop().s8;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.stack_push(div_overflow(dividend, divisor));
            break;
        }

            /* ======================= Conversions ======================= */
        case OpCodes::l2i: {
            auto value = frame.stack_pop().s4;
            frame.stack_push(value);
            break;
        }

            /* ======================= Comparisons ======================= */
        case OpCodes::lcmp: {
            auto b = frame.stack_pop().s8;
            auto a = frame.stack_pop().s8;
            s8 diff = a - b;
            frame.stack_push(std::clamp(diff, static_cast<s8>(-1), static_cast<s8>(1)));
            break;
        }
        case OpCodes::ifeq:
            return execute_comparison(code, pc, frame.stack_pop().s4 == 0);
        case OpCodes::ifne:
            return execute_comparison(code, pc, frame.stack_pop().s4 != 0);
        case OpCodes::iflt:
            return execute_comparison(code, pc, frame.stack_pop().s4 < 0);
        case OpCodes::ifge:
            return execute_comparison(code, pc, frame.stack_pop().s4 >= 0);
        case OpCodes::ifgt:
            return execute_comparison(code, pc, frame.stack_pop().s4 > 0);
        case OpCodes::ifle:
            return execute_comparison(code, pc, frame.stack_pop().s4 <= 0);
        case OpCodes::if_icmpeq:
            return execute_comparison(code, pc, frame.stack_pop().s4 == frame.stack_pop().s4);
        case OpCodes::if_icmpne:
            return execute_comparison(code, pc, frame.stack_pop().s4 != frame.stack_pop().s4);
        case OpCodes::if_icmplt:
            return execute_comparison(code, pc, frame.stack_pop().s4 > frame.stack_pop().s4);
        case OpCodes::if_icmpge:
            return execute_comparison(code, pc, frame.stack_pop().s4 <= frame.stack_pop().s4);
        case OpCodes::if_icmpgt:
            return execute_comparison(code, pc, frame.stack_pop().s4 < frame.stack_pop().s4);
        case OpCodes::if_icmple:
            return execute_comparison(code, pc, frame.stack_pop().s4 >= frame.stack_pop().s4);
//        case OpCodes::if_acmpeq:
//            return execute_comparison(code, pc, frame.stack_pop().u8 == frame.stack_pop().u8);
//        case OpCodes::if_acmpne:
//            return execute_comparison(code, pc, frame.stack_pop().u8 != frame.stack_pop().u8);

            /* ======================= Control =======================*/
        case OpCodes::goto_:
            return goto_(code, pc);
        case OpCodes::return_:
            shouldExit = -1;
            break;


            /* ======================= References ======================= */
        case OpCodes::invokestatic: {
            size_t method_index = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
            // TODO this is really an index into the run-time constant pool
            auto method = std::get<CONSTANT_Methodref_info>(frame.clas.constant_pool.table[method_index].variant);

            // TODO this is hardcoded for now
            if (method.class_->name->value == "java/lang/System" && method.name_and_type->name->value == "exit" &&
                method.name_and_type->descriptor->value == "(I)V") {
                shouldExit = true;
            } else if (method.name_and_type->name->value == "println" &&
                       method.name_and_type->descriptor->value == "(I)V") {
                std::cout << frame.stack_pop().s4 << "\n";
            } else if (method.name_and_type->name->value == "println" &&
                       method.name_and_type->descriptor->value == "(J)V") {
                std::cout << frame.stack_pop().s8 << "\n";
            } else {
                throw std::runtime_error("Unimplemented invokestatic: " + method.name_and_type->name->value +
                                         method.name_and_type->descriptor->value);
            }
            return pc + 3;
        }

        default:
            throw std::runtime_error(
                    "Unimplemented/unknown opcode " + std::to_string(opcode) + " at " + std::to_string(pc)
            );
    }

    return pc + 1;
}

static size_t execute_comparison(const std::vector<u1> &code, size_t pc, bool condition) {
    if (condition) {
        return goto_(code, pc);
    } else {
        return pc + 3;
    }
}

static size_t goto_(const std::vector<u1> &code, size_t pc) {
    u2 offset_u = static_cast<u2>(static_cast<u2>(code[pc + 1]) << 8 | static_cast<u2>(code[pc + 2]));
    auto offset = future::bit_cast<s2>(offset_u);
    return static_cast<size_t>(static_cast<long>(pc) + offset);
}