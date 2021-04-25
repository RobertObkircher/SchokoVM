#include "interpreter.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <iostream>
#include <type_traits>
#include <cmath>
#include "opcodes.hpp"
#include "future.hpp"

static const u2 MAIN_ACCESS_FLAGS = (static_cast<u2>(FieldInfoAccessFlags::ACC_PUBLIC) |
                                     static_cast<u2>(FieldInfoAccessFlags::ACC_STATIC));
static const auto MAIN_NAME = "main";
static const auto MAIN_DESCRIPTOR = "([Ljava/lang/String;)V";

static size_t execute_instruction(Thread &thread, Frame &frame,
                                  std::unordered_map<std::string_view, ClassFile *> &class_files, bool &shouldExit);

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
    return future::bit_cast<T>(
            future::bit_cast<std::make_unsigned_t<T>>(a) - future::bit_cast<std::make_unsigned_t<T>>(b)
    );
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

template<std::floating_point F, std::signed_integral I>
static inline I floating_to_integer(F f) {
    if (std::isnan(f)) {
        return 0;
    } else if (f > static_cast<F>(std::numeric_limits<I>::max())) {
        return std::numeric_limits<I>::max();
    } else if (f < static_cast<F>(std::numeric_limits<I>::min())) {
        return std::numeric_limits<I>::min();
    } else {
        return static_cast<I>(f);
    }
}

int interpret(std::unordered_map<std::string_view, ClassFile *> &class_files, ClassFile *main) {
    auto main_method_iter = std::find_if(main->methods.begin(), main->methods.end(),
                                         [](const method_info &m) {
                                             return m.name_index->value == MAIN_NAME &&
                                                    m.descriptor_index->value == MAIN_DESCRIPTOR &&
                                                    (m.access_flags & MAIN_ACCESS_FLAGS) == MAIN_ACCESS_FLAGS;
                                         }
    );
    if (main_method_iter == std::end(main->methods)) {
        throw std::runtime_error("Couldn't find main method");
    }
    method_info *main_method = &*main_method_iter;

    Thread thread;
    thread.stack.memory.resize(1024 * 1024 / sizeof(Value)); // 1mb for now

    thread.stack.memory_used = 1;
    thread.stack.memory[0] = Value(0); // TODO args[] for main

    Frame frame{thread.stack, main, main_method, thread.stack.memory_used};

    bool shouldExit = false;
    while (!shouldExit) {
        frame.pc = execute_instruction(thread, frame, class_files, shouldExit);
    }

    // print exit code
    if (frame.operands_count == 0) {
        return 0;
    } else {
        return frame.stack_pop().s4;
    }
}

static inline size_t execute_instruction(Thread &thread, Frame &frame,
                                         std::unordered_map<std::string_view, ClassFile *> &class_files,
                                         bool &shouldExit) {
    size_t pc = frame.pc;
    std::vector<u1> &code = *frame.code;
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
            u2 value = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            frame.stack_push(static_cast<s4>(future::bit_cast<s2>(value)));
            return pc + 3;
        }
// TODO the following instructions actually read from the runtime constant pool
        case OpCodes::ldc: {
            auto index = code[pc + 1];
            auto &entry = frame.clazz->constant_pool.table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.stack_push(i->bytes);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.stack_push(f->value);
            } else {
                throw std::runtime_error("ldc refers to invalid/unimplemented type");
            }
            return pc + 2;
        }
        case OpCodes::ldc_w: {
            size_t index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto &entry = frame.clazz->constant_pool.table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.stack_push(i->bytes);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.stack_push(f->value);
            } else {
                throw std::runtime_error("ldc_w refers to invalid/unimplemented type");
            }
            return pc + 3;
        }
        case OpCodes::ldc2_w: {
            size_t index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto &entry = frame.clazz->constant_pool.table[index];
            if (auto l = std::get_if<CONSTANT_Long_info>(&entry.variant)) {
                frame.stack_push(l->value);
            } else if (auto d = std::get_if<CONSTANT_Double_info>(&entry.variant)) {
                frame.stack_push(d->value);
            } else {
                throw std::runtime_error("ldc2_w refers to invalid/unimplemented type");
            }
            return pc + 3;
        }

            /* ======================= Loads ======================= */
            // In a slight derivation from the spec, longs and doubles are stored in a single local
        case OpCodes::iload:
        case OpCodes::lload:
        case OpCodes::fload:
        case OpCodes::dload: {

            auto local = code[pc + 1];
            frame.stack_push(frame.locals[local]);
            return pc + 2;
        }
//        case OpCodes::aload:
        case OpCodes::iload_0:
        case OpCodes::iload_1:
        case OpCodes::iload_2:
        case OpCodes::iload_3: {
            auto value = frame.locals[opcode - static_cast<u1>(OpCodes::iload_0)].s4;
            frame.stack_push(value);
            break;
        }
        case OpCodes::lload_0:
        case OpCodes::lload_1:
        case OpCodes::lload_2:
        case OpCodes::lload_3: {
            // In a slight derivation from the spec, longs are stored in a single local
            u1 index = static_cast<u1>(opcode - static_cast<u1>(OpCodes::lload_0));
            frame.stack_push(frame.locals[index].s8);
            break;
        }
        case OpCodes::fload_0:
        case OpCodes::fload_1:
        case OpCodes::fload_2:
        case OpCodes::fload_3: {
            auto value = frame.locals[opcode - static_cast<u1>(OpCodes::fload_0)].float_;
            frame.stack_push(value);
            break;
        }
        case OpCodes::dload_0:
        case OpCodes::dload_1:
        case OpCodes::dload_2:
        case OpCodes::dload_3: {
            // In a slight derivation from the spec, double are stored in a single local
            u1 index = static_cast<u1>(opcode - static_cast<u1>(OpCodes::dload_0));
            frame.stack_push(frame.locals[index].double_);
            break;
        }

            /* ======================= Stores ======================= */
            // In a slight derivation from the spec, longs and doubles are stored in a single local
        case OpCodes::istore:
        case OpCodes::lstore:
        case OpCodes::fstore:
        case OpCodes::dstore:
            frame.locals[code[pc + 1]] = frame.stack_pop();
            return pc + 2;
        case OpCodes::istore_0:
        case OpCodes::istore_1:
        case OpCodes::istore_2:
        case OpCodes::istore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::istore_0)] = {frame.stack_pop().s4};
            break;
        case OpCodes::lstore_0:
        case OpCodes::lstore_1:
        case OpCodes::lstore_2:
        case OpCodes::lstore_3: {
            // In a slight derivation from the spec, longs are stored in a single local
            auto value = frame.stack_pop().s8;
            frame.locals[opcode - static_cast<u1>(OpCodes::lstore_0)] = {value};
            break;
        }
        case OpCodes::fstore_0:
        case OpCodes::fstore_1:
        case OpCodes::fstore_2:
        case OpCodes::fstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::fstore_0)] = {frame.stack_pop().float_};
            break;
        case OpCodes::dstore_0:
        case OpCodes::dstore_1:
        case OpCodes::dstore_2:
        case OpCodes::dstore_3: {
            // In a slight derivation from the spec, double are stored in a single local
            frame.locals[opcode - static_cast<u1>(OpCodes::dstore_0)] = {frame.stack_pop().double_};
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
        case OpCodes::fadd: {
            auto b = frame.stack_pop().float_;
            auto a = frame.stack_pop().float_;
            frame.stack_push(a + b);
            break;
        }
        case OpCodes::dadd: {
            auto b = frame.stack_pop().double_;
            auto a = frame.stack_pop().double_;
            frame.stack_push(a + b);
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
        case OpCodes::fsub: {
            auto b = frame.stack_pop().float_;
            auto a = frame.stack_pop().float_;
            frame.stack_push(a - b);
            break;
        }
        case OpCodes::dsub: {
            auto b = frame.stack_pop().double_;
            auto a = frame.stack_pop().double_;
            frame.stack_push(a - b);
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
        case OpCodes::fmul: {
            auto a = frame.stack_pop().float_;
            auto b = frame.stack_pop().float_;
            frame.stack_push(a * b);
            break;
        }
        case OpCodes::dmul: {
            auto a = frame.stack_pop().double_;
            auto b = frame.stack_pop().double_;
            frame.stack_push(a * b);
            break;
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
        case OpCodes::fdiv: {
            auto divisor = frame.stack_pop().float_;
            auto dividend = frame.stack_pop().float_;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.stack_push(dividend / divisor);
            break;
        }
        case OpCodes::ddiv: {
            auto divisor = frame.stack_pop().double_;
            auto dividend = frame.stack_pop().double_;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.stack_push(dividend / divisor);
            break;
        }

        case OpCodes::irem: {
            auto divisor = frame.stack_pop().s4;
            auto dividend = frame.stack_pop().s4;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = dividend - mul_overflow(div_overflow(dividend, divisor), divisor);
            frame.stack_push(result);
            break;
        }
        case OpCodes::lrem: {
            auto divisor = frame.stack_pop().s8;
            auto dividend = frame.stack_pop().s8;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = dividend - mul_overflow(div_overflow(dividend, divisor), divisor);
            frame.stack_push(result);
            break;
        }
        case OpCodes::frem: {
            auto divisor = frame.stack_pop().float_;
            auto dividend = frame.stack_pop().float_;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = std::fmod(dividend, divisor);
            frame.stack_push(result);
            break;
        }
        case OpCodes::drem: {
            auto divisor = frame.stack_pop().double_;
            auto dividend = frame.stack_pop().double_;
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = std::fmod(dividend, divisor);
            frame.stack_push(result);
            break;
        }
        case OpCodes::ineg: {
            auto a = frame.stack_pop().s4;
            frame.stack_push(sub_overflow(static_cast<s4>(0), a));
            break;
        }
        case OpCodes::lneg: {
            auto a = frame.stack_pop().s8;
            frame.stack_push(sub_overflow(static_cast<s8>(0), a));
            break;
        }
        case OpCodes::fneg: {
            auto a = frame.stack_pop().float_;
            frame.stack_push(-a);
            break;
        }
        case OpCodes::dneg: {
            auto a = frame.stack_pop().double_;
            frame.stack_push(-a);
            break;
        }
        case OpCodes::ishl: {
            auto shift = frame.stack_pop().s4 & 0x1F;
            auto value = frame.stack_pop().s4;
            frame.stack_push(value << shift);
            break;
        }
        case OpCodes::lshl: {
            auto shift = frame.stack_pop().s4 & 0x3F;
            auto value = frame.stack_pop().s8;
            frame.stack_push(value << shift);
            break;
        }
        case OpCodes::ishr: {
            auto shift = frame.stack_pop().s4 & 0x1F;
            auto value = frame.stack_pop().s4;
            frame.stack_push(value >> shift);
            break;
        }
        case OpCodes::lshr: {
            auto shift = frame.stack_pop().s4 & 0x3F;
            auto value = frame.stack_pop().s8;
            frame.stack_push(value >> shift);
            break;
        }
        case OpCodes::iushr: {
            auto shift = frame.stack_pop().s4 & 0x1F;
            auto value = frame.stack_pop().s4;
            // C++20 always performs arithmetic shifts, so the top bits need to be cleared out afterwards
            frame.stack_push((value >> shift) &
                             (future::bit_cast<s4>(std::numeric_limits<u4>::max() >> static_cast<u4>(shift)))
            );
            break;
        }
        case OpCodes::lushr: {
            auto shift = frame.stack_pop().s4 & 0x3F;
            auto value = frame.stack_pop().s8;
            // C++20 always performs arithmetic shifts, so the top bits need to be cleared out afterwards
            frame.stack_push((value >> shift) &
                             (future::bit_cast<s8>(std::numeric_limits<u8>::max() >> static_cast<u8>(shift)))
            );
            break;
        }
        case OpCodes::iand:
            frame.stack_push(frame.stack_pop().s4 & frame.stack_pop().s4);
            break;
        case OpCodes::land:
            frame.stack_push(frame.stack_pop().s8 & frame.stack_pop().s8);
            break;
        case OpCodes::ior:
            frame.stack_push(frame.stack_pop().s4 | frame.stack_pop().s4);
            break;
        case OpCodes::lor:
            frame.stack_push(frame.stack_pop().s8 | frame.stack_pop().s8);
            break;
        case OpCodes::ixor:
            frame.stack_push(frame.stack_pop().s4 ^ frame.stack_pop().s4);
            break;
        case OpCodes::lxor:
            frame.stack_push(frame.stack_pop().s8 ^ frame.stack_pop().s8);
            break;
        case OpCodes::iinc: {
            auto local = code[pc + 1];
            auto value = static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(code[pc + 2])));
            auto result = add_overflow(frame.locals[local].s4, value);
            frame.locals[local] = {result};
            return pc + 3;
        }

            /* ======================= Conversions ======================= */
        case OpCodes::i2l:
            frame.stack_push(static_cast<s8>(frame.stack_pop().s4));
            break;
        case OpCodes::i2f:
            frame.stack_push(static_cast<float>(frame.stack_pop().s4));
            break;
        case OpCodes::i2d:
            frame.stack_push(static_cast<double>(frame.stack_pop().s4));
            break;
        case OpCodes::l2i:
            frame.stack_push(static_cast<s4>(frame.stack_pop().s8));
            break;
        case OpCodes::l2f:
            frame.stack_push(static_cast<float>(frame.stack_pop().s8));
            break;
        case OpCodes::l2d:
            frame.stack_push(static_cast<double>(frame.stack_pop().s8));
            break;
        case OpCodes::f2i: {
            frame.stack_push(floating_to_integer<float, s4>(frame.stack_pop().float_));
            break;
        }
        case OpCodes::f2l:
            frame.stack_push(floating_to_integer<float, s8>(frame.stack_pop().float_));
            break;
        case OpCodes::f2d:
            frame.stack_push(static_cast<double>(frame.stack_pop().float_));
            break;
        case OpCodes::d2i:
            frame.stack_push(floating_to_integer<double, s4>(frame.stack_pop().double_));
            break;
        case OpCodes::d2l:
            frame.stack_push(floating_to_integer<double, s8>(frame.stack_pop().double_));
            break;
        case OpCodes::d2f:
            frame.stack_push(static_cast<float>(frame.stack_pop().double_));
            break;
        case OpCodes::i2b:
            frame.stack_push(static_cast<s4>(static_cast<s1>(frame.stack_pop().s4)));
            break;
        case OpCodes::i2c:
            frame.stack_push(static_cast<s4>(static_cast<u2>(frame.stack_pop().s4)));
            break;
        case OpCodes::i2s:
            frame.stack_push(static_cast<s4>(static_cast<s2>(frame.stack_pop().s4)));
            break;

            /* ======================= Comparisons ======================= */
        case OpCodes::lcmp: {
            auto b = frame.stack_pop().s8;
            auto a = frame.stack_pop().s8;
            if (a > b) {
                frame.stack_push(1);
            } else if (a == b) {
                frame.stack_push(0);
            } else {
                frame.stack_push(-1);
            }
            break;
        }
        case OpCodes::fcmpl:
        case OpCodes::fcmpg: {
            // TODO "value set conversion" ?
            auto b = frame.stack_pop().float_;
            auto a = frame.stack_pop().float_;
            if (a > b) {
                frame.stack_push(1);
            } else if (a == b) {
                frame.stack_push(0);
            } else if (a < b) {
                frame.stack_push(-1);
            } else {
                // at least one of a' or b' is NaN
                frame.stack_push(static_cast<OpCodes>(opcode) == OpCodes::fcmpg ? -1 : 1);
            }
            break;
        }
        case OpCodes::dcmpl:
        case OpCodes::dcmpg: {
            // TODO "value set conversion" ?
            auto b = frame.stack_pop().double_;
            auto a = frame.stack_pop().double_;
            if (a > b) {
                frame.stack_push(1);
            } else if (a == b) {
                frame.stack_push(0);
            } else if (a < b) {
                frame.stack_push(-1);
            } else {
                // at least one of a' or b' is NaN
                frame.stack_push(static_cast<OpCodes>(opcode) == OpCodes::dcmpl ? -1 : 1);
            }
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
//            return execute_comparison(code, pc, frame.stack_pop().u8() == frame.stack_pop().u8());
//        case OpCodes::if_acmpne:
//            return execute_comparison(code, pc, frame.stack_pop().u8() != frame.stack_pop().u8());

            /* ======================= Control =======================*/
        case OpCodes::goto_:
            return goto_(code, pc);
        case OpCodes::tableswitch: {
            size_t opcode_address = pc;

            // skip 0-3 bytes of padding
            if ((pc % 4) != 0)
                pc += 4 - pc % 4;

            s4 default_ = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 low = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 high = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            assert(low <= high);

//            s4 count  = high - low + 1;
            s4 index = frame.stack_pop().s4;

            s4 offset;
            if (index < low || index > high) {
                offset = default_;
            } else {
                pc += 4 * static_cast<u4>(index - low);
                offset = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            }

            return opcode_address + static_cast<size_t>(static_cast<ssize_t>(offset));
        }
        case OpCodes::lookupswitch: {
            size_t opcode_address = pc;

            // skip 0-3 bytes of padding
            if ((pc % 4) != 0)
                pc += 4 - pc % 4;

            s4 default_ = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 npairs = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            assert(npairs >= 0);

            s4 key = frame.stack_pop().s4;
            s4 offset = default_;

            for (s4 i = 0; i < npairs; ++i) {
                s4 match = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) |
                                           code[pc + 3]);
                pc += 4;
                if (key == match) {
                    offset = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) |
                                             code[pc + 3]);
                    break;
                }
                pc += 4;
            }

            return opcode_address + static_cast<size_t>(static_cast<ssize_t>(offset));
        }

        case OpCodes::ireturn:
        case OpCodes::lreturn:
        case OpCodes::freturn:
        case OpCodes::dreturn:
        case OpCodes::areturn:
            frame.locals[0] = frame.stack_pop();
            // fallthrough
        case OpCodes::return_: {
            if (thread.stack.parent_frames.empty()) {
                shouldExit = true;
            } else {
                thread.stack.memory_used = frame.previous_stack_memory_usage;
                frame = thread.stack.parent_frames[thread.stack.parent_frames.size() - 1];
                thread.stack.parent_frames.pop_back();
                return frame.pc;
            }
            break;
        }


            /* ======================= References ======================= */
        case OpCodes::invokestatic: {
            size_t method_index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto method_ref = std::get<CONSTANT_Methodref_info>(frame.clazz->constant_pool.table[method_index].variant);

            // TODO this is hardcoded for now
            if (method_ref.class_->name->value == "java/lang/System" &&
                method_ref.name_and_type->name->value == "exit" &&
                method_ref.name_and_type->descriptor->value == "(I)V") {
                shouldExit = true;
                return pc;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(I)V") {
                std::cout << frame.stack_pop().s4 << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(J)V") {
                std::cout << frame.stack_pop().s8 << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(F)V") {
                // floatToIntBits
                float f = frame.stack_pop().float_;
                s4 i = future::bit_cast<s4>(f);
                if (std::isnan(f)) {
                    // canonical NaN
                    i = future::bit_cast<s4>(0x7fc00000);
                }
                std::cout << i << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(D)V") {
                // doubleToLongBits
                double d = frame.stack_pop().double_;
                s8 i = future::bit_cast<s8>(d);
                if (std::isnan(d)) {
                    // canonical NaN
                    i = future::bit_cast<s8>(0x7ff8000000000000L);
                }
                std::cout << i << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(C)V") {
                std::cout << frame.stack_pop().s4 << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(S)V") {
                std::cout << frame.stack_pop().s4 << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(B)V") {
                std::cout << frame.stack_pop().s4 << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(Z)V") {
                auto value = frame.stack_pop().s4;
                if (value == 1) {
                    std::cout << "true\n";
                } else if (value == 0) {
                    std::cout << "false\n";
                } else {
                    abort();
                }
                return pc + 3;
            }

            method_info *method = method_ref.method;
            ClassFile *clazz = method_ref.class_->clazz;

            if (method == nullptr) {
                if (clazz == nullptr) {
                    auto result = class_files.find(method_ref.class_->name->value);
                    if (result == class_files.end()) {
                        throw std::runtime_error("class not found: '" + method_ref.class_->name->value + "'");
                    }
                    method_ref.class_->clazz = clazz = result->second;

                    // TODO if clazz is not initialized: create a stackframe with the initializer but do not advance the pc of this frame
                }

                auto method_iter = std::find_if(clazz->methods.begin(), clazz->methods.end(),
                                                [method_ref](const method_info &m) {
                                                    return m.name_index->value ==
                                                           method_ref.name_and_type->name->value &&
                                                           m.descriptor_index->value ==
                                                           method_ref.name_and_type->descriptor->value
                                                        // TODO m.access_flags
                                                            ;
                                                });
                if (method_iter == std::end(clazz->methods)) {
                    throw std::runtime_error(
                            "Couldn't find method_ref: '" + method_ref.name_and_type->name->value + "' " +
                            method_ref.name_and_type->descriptor->value + " in class " +
                            method_ref.class_->name->value);
                }

                method_ref.method = method = &*method_iter;
            }

            if (method->access_flags & static_cast<u2>(MethodInfoAccessFlags::ACC_NATIVE)) {
                abort();
                return pc + 3;
            } else {
                size_t operand_stack_top = frame.first_operand_index + frame.operands_count;
                frame.operands_count += method->minus_parameter_count_plus_return_count;

                frame.pc += 3;
                thread.stack.parent_frames.push_back(frame);

                frame = {thread.stack, clazz, method, operand_stack_top};
                if (thread.stack.memory_used > thread.stack.memory.size())
                    throw std::runtime_error("stack overflow");
                return 0; // will be set on the new frame
            }
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
    u2 offset_u = static_cast<u2>((code[pc + 1]) << 8 | (code[pc + 2]));
    auto offset = future::bit_cast<s2>(offset_u);
    return static_cast<size_t>(static_cast<long>(pc) + offset);
}

Frame::Frame(Stack &stack, ClassFile *clazz, method_info *method, size_t operand_stack_top)
        : clazz(clazz),
          method(method),
          code(&method->code_attribute->code),
          operands_count(0),
          previous_stack_memory_usage(stack.memory_used),
          pc(0) {
    //assert(method->code_attribute->max_locals >= method->parameter_count);
    assert(stack.memory_used >= method->parameter_count);
    assert(operand_stack_top >= method->parameter_count);

    size_t first_local_index = operand_stack_top - method->parameter_count;
    first_operand_index = first_local_index + method->code_attribute->max_locals;
    stack.memory_used = first_operand_index + method->code_attribute->max_stack;

    locals = {&stack.memory[first_local_index], method->code_attribute->max_locals};
    operands = {&stack.memory[first_operand_index], method->code_attribute->max_stack};

    if (method->move_arguments) {
        size_t target = method->stack_slots_used_by_parameters;
        size_t source = method->parameter_count;
        assert(target > source); // move_arguments is false if they are equal
        do {
            --target;
            --source;
            if (method->argument_takes_two_local_variables[source]) {
                --target;
                if (source == target) break;
            }
            locals[target] = locals[source];
        } while (source != 0);
    }

    // TODO think about value set conversion
    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.8.3
}
