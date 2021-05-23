#include "interpreter.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <iostream>
#include <cmath>
#include "opcodes.hpp"
#include "future.hpp"
#include "math.hpp"
#include "classloading.hpp"

static const u2 MAIN_ACCESS_FLAGS = (static_cast<u2>(FieldInfoAccessFlags::ACC_PUBLIC) |
                                     static_cast<u2>(FieldInfoAccessFlags::ACC_STATIC));
static const auto MAIN_NAME = "main";
static const auto MAIN_DESCRIPTOR = "([Ljava/lang/String;)V";

enum class ArrayPrimitiveTypes {
    T_BOOLEAN = 4,
    T_CHAR = 5,
    T_FLOAT = 6,
    T_DOUBLE = 7,
    T_BYTE = 8,
    T_SHORT = 9,
    T_INT = 10,
    T_LONG = 11,
};

static size_t execute_instruction(Heap &heap, Thread &thread, Frame &frame,
                                  std::unordered_map<std::string_view, ClassFile *> &class_files, bool &shouldExit);

static size_t execute_comparison(const std::vector<u1> &code, size_t pc, bool condition);

static size_t goto_(const std::vector<u1> &code, size_t pc);

static size_t handle_throw(Thread &thread, Frame &frame, bool &shouldExit, size_t &pc, Reference exception);

static inline size_t pop_frame(Thread &thread, Frame &frame);

template<typename Element>
size_t array_store(Frame &frame, size_t pc);

template<typename Element>
size_t array_load(Frame &frame, size_t pc);

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

    Heap heap{};

    Thread thread{};
    thread.stack.memory.resize(1024 * 1024 / sizeof(Value)); // 1mb for now

    thread.stack.memory_used = 1;
    thread.stack.memory[0] = Value(0); // TODO args[] for main

    Frame frame{thread.stack, main, main_method, thread.stack.memory_used};

    // push the class initializer if necessary
    ClassResolution resolved = resolve_class(class_files, main->this_class, thread, frame);
    if (resolved == ClassResolution::NOT_FOUND)
        abort();

    bool shouldExit = false;
    while (!shouldExit) {
        frame.pc = execute_instruction(heap, thread, frame, class_files, shouldExit);
    }

    // print exit code
    if (frame.operands_top == 0) {
        return 0;
    } else {
        return frame.pop_s4();
    }
}

static inline size_t execute_instruction(Heap &heap, Thread &thread, Frame &frame,
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
        case OpCodes::aconst_null:
            frame.push_a(JAVA_NULL);
            break;
        case OpCodes::iconst_m1:
        case OpCodes::iconst_0:
        case OpCodes::iconst_1:
        case OpCodes::iconst_2:
        case OpCodes::iconst_3:
        case OpCodes::iconst_4:
        case OpCodes::iconst_5:
            frame.push_s4(opcode - static_cast<u1>(OpCodes::iconst_0));
            break;

        case OpCodes::lconst_0:
        case OpCodes::lconst_1:
            frame.push_s8(opcode - static_cast<u1>(OpCodes::lconst_0));
            break;
        case OpCodes::fconst_0:
        case OpCodes::fconst_1:
        case OpCodes::fconst_2:
            frame.push_f(static_cast<float>(opcode - static_cast<u1>(OpCodes::fconst_0)));
            break;
        case OpCodes::dconst_0:
        case OpCodes::dconst_1:
            frame.push_d(static_cast<double>(opcode - static_cast<u1>(OpCodes::dconst_0)));
            break;
        case OpCodes::bipush: {
            frame.push_s4(static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(code[pc + 1]))));
            return pc + 2;
        }
        case OpCodes::sipush: {
            u2 value = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            frame.push_s4(future::bit_cast<s2>(value));
            return pc + 3;
        }
        case OpCodes::ldc: {
            auto index = code[pc + 1];
            auto &entry = frame.clazz->constant_pool.table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.push_s4(i->value);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.push_f(f->value);
            } else {
                throw std::runtime_error("ldc refers to invalid/unimplemented type");
            }
            return pc + 2;
        }
        case OpCodes::ldc_w: {
            size_t index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto &entry = frame.clazz->constant_pool.table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.push_s4(i->value);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.push_f(f->value);
            } else {
                throw std::runtime_error("ldc_w refers to invalid/unimplemented type");
            }
            return pc + 3;
        }
        case OpCodes::ldc2_w: {
            size_t index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto &entry = frame.clazz->constant_pool.table[index];
            if (auto l = std::get_if<CONSTANT_Long_info>(&entry.variant)) {
                frame.push_s8(l->value);
            } else if (auto d = std::get_if<CONSTANT_Double_info>(&entry.variant)) {
                frame.push_d(d->value);
            } else {
                throw std::runtime_error("ldc2_w refers to invalid/unimplemented type");
            }
            return pc + 3;
        }

            /* ======================= Loads ======================= */
        case OpCodes::iload:
            frame.push_s4(frame.locals[code[pc + 1]].s4);
            return pc + 2;
        case OpCodes::lload:
            frame.push_s8(frame.locals[code[pc + 1]].s8);
            return pc + 2;
        case OpCodes::fload:
            frame.push_f(frame.locals[code[pc + 1]].float_);
            return pc + 2;
        case OpCodes::dload:
            frame.push_d(frame.locals[code[pc + 1]].double_);
            return pc + 2;
        case OpCodes::aload:
            frame.push_a(frame.locals[code[pc + 1]].reference);
            return pc + 2;

        case OpCodes::iload_0:
        case OpCodes::iload_1:
        case OpCodes::iload_2:
        case OpCodes::iload_3:
            frame.push_s4(frame.locals[opcode - static_cast<u1>(OpCodes::iload_0)].s4);
            break;
        case OpCodes::lload_0:
        case OpCodes::lload_1:
        case OpCodes::lload_2:
        case OpCodes::lload_3:
            frame.push_s8(frame.locals[(static_cast<u1>(opcode - static_cast<u1>(OpCodes::lload_0)))].s8);
            break;
        case OpCodes::fload_0:
        case OpCodes::fload_1:
        case OpCodes::fload_2:
        case OpCodes::fload_3:
            frame.push_f(frame.locals[opcode - static_cast<u1>(OpCodes::fload_0)].float_);
            break;
        case OpCodes::dload_0:
        case OpCodes::dload_1:
        case OpCodes::dload_2:
        case OpCodes::dload_3:
            frame.push_d(frame.locals[static_cast<u1>(opcode - static_cast<u1>(OpCodes::dload_0))].double_);
            break;
        case OpCodes::aload_0:
        case OpCodes::aload_1:
        case OpCodes::aload_2:
        case OpCodes::aload_3:
            frame.push_a(frame.locals[static_cast<u1>(opcode - static_cast<u1>(OpCodes::aload_0))].reference);
            break;
        case OpCodes::iaload:
            return array_load<s4>(frame, pc);
        case OpCodes::laload:
            return array_load<s8>(frame, pc);
        case OpCodes::faload:
            return array_load<float>(frame, pc);
        case OpCodes::daload:
            return array_load<double>(frame, pc);
        case OpCodes::aaload:
            return array_load<Reference>(frame, pc);
        case OpCodes::baload:
            return array_load<s1>(frame, pc);
        case OpCodes::caload:
            return array_load<u2>(frame, pc);
        case OpCodes::saload:
            return array_load<s4>(frame, pc);

            /* ======================= Stores ======================= */
        case OpCodes::istore:
            frame.locals[code[pc + 1]] = Value(frame.pop_s4());
            return pc + 2;
        case OpCodes::lstore:
            frame.locals[code[pc + 1]] = Value(frame.pop_s8());
            return pc + 2;
        case OpCodes::fstore:
            frame.locals[code[pc + 1]] = Value(frame.pop_f());
            return pc + 2;
        case OpCodes::dstore:
            frame.locals[code[pc + 1]] = Value(frame.pop_d());
            return pc + 2;
        case OpCodes::astore:
            frame.locals[code[pc + 1]] = Value(frame.pop_a());
            return pc + 2;
        case OpCodes::istore_0:
        case OpCodes::istore_1:
        case OpCodes::istore_2:
        case OpCodes::istore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::istore_0)] = Value(frame.pop_s4());
            break;
        case OpCodes::lstore_0:
        case OpCodes::lstore_1:
        case OpCodes::lstore_2:
        case OpCodes::lstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::lstore_0)] = Value(frame.pop_s8());
            break;
        case OpCodes::fstore_0:
        case OpCodes::fstore_1:
        case OpCodes::fstore_2:
        case OpCodes::fstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::fstore_0)] = Value(frame.pop_f());
            break;
        case OpCodes::dstore_0:
        case OpCodes::dstore_1:
        case OpCodes::dstore_2:
        case OpCodes::dstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::dstore_0)] = Value(frame.pop_d());
            break;
        case OpCodes::astore_0:
        case OpCodes::astore_1:
        case OpCodes::astore_2:
        case OpCodes::astore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::astore_0)] = Value(frame.pop_a());
            break;
        case OpCodes::iastore:
            return array_store<s4>(frame, pc);
        case OpCodes::lastore:
            return array_store<s8>(frame, pc);
        case OpCodes::fastore:
            return array_store<float>(frame, pc);
        case OpCodes::dastore:
            return array_store<double>(frame, pc);
        case OpCodes::aastore:
            return array_store<Reference>(frame, pc);
        case OpCodes::bastore:
            return array_store<s1>(frame, pc);
        case OpCodes::castore:
            return array_store<u2>(frame, pc);
        case OpCodes::sastore:
            return array_store<s4>(frame, pc);

            /* ======================= Stack =======================*/
        case OpCodes::pop:
            frame.pop();
            break;
        case OpCodes::pop2:
            frame.pop2();
            break;
        case OpCodes::dup: {
            auto value = frame.pop();
            frame.push(value);
            frame.push(value);
            break;
        }
        case OpCodes::dup_x1: {
            auto value1 = frame.pop();
            auto value2 = frame.pop();
            frame.push(value1);
            frame.push(value2);
            frame.push(value1);
            break;
        }
        case OpCodes::dup_x2: {
            auto value1 = frame.pop();
            auto value2 = frame.pop();
            auto value3 = frame.pop();
            frame.push(value1);
            frame.push(value3);
            frame.push(value2);
            frame.push(value1);
            break;
        }
        case OpCodes::dup2: {
            auto value1 = frame.pop();
            auto value2 = frame.pop();
            frame.push(value2);
            frame.push(value1);
            frame.push(value2);
            frame.push(value1);
            break;
        }
        case OpCodes::dup2_x1: {
            auto value1 = frame.pop();
            auto value2 = frame.pop();
            auto value3 = frame.pop();
            frame.push(value2);
            frame.push(value1);
            frame.push(value3);
            frame.push(value2);
            frame.push(value1);
            break;
        }
        case OpCodes::dup2_x2: {
            auto value1 = frame.pop();
            auto value2 = frame.pop();
            auto value3 = frame.pop();
            auto value4 = frame.pop();
            frame.push(value2);
            frame.push(value1);
            frame.push(value4);
            frame.push(value3);
            frame.push(value2);
            frame.push(value1);
            break;
        }
        case OpCodes::swap: {
            auto value1 = frame.pop();
            auto value2 = frame.pop();
            frame.push(value1);
            frame.push(value2);
            break;
        }

            /* ======================= Math =======================*/
        case OpCodes::iadd: {
            auto b = frame.pop_s4();
            auto a = frame.pop_s4();
            auto result = add_overflow(a, b);
            frame.push_s4(result);
            break;
        }
        case OpCodes::ladd: {
            auto b = frame.pop_s8();
            auto a = frame.pop_s8();
            auto result = add_overflow(a, b);
            frame.push_s8(result);
            break;
        }
        case OpCodes::fadd: {
            auto b = frame.pop_f();
            auto a = frame.pop_f();
            frame.push_f(a + b);
            break;
        }
        case OpCodes::dadd: {
            auto b = frame.pop_d();
            auto a = frame.pop_d();
            frame.push_d(a + b);
            break;
        }
        case OpCodes::isub: {
            auto b = frame.pop_s4();
            auto a = frame.pop_s4();
            auto result = sub_overflow(a, b);
            frame.push_s4(result);
            break;
        }
        case OpCodes::lsub: {
            s8 b = frame.pop_s8();
            s8 a = frame.pop_s8();
            frame.push_s8(sub_overflow(a, b));
            break;
        }
        case OpCodes::fsub: {
            auto b = frame.pop_f();
            auto a = frame.pop_f();
            frame.push_f(a - b);
            break;
        }
        case OpCodes::dsub: {
            auto b = frame.pop_d();
            auto a = frame.pop_d();
            frame.push_d(a - b);
            break;
        }
        case OpCodes::imul: {
            auto a = frame.pop_s4();
            auto b = frame.pop_s4();
            frame.push_s4(mul_overflow(a, b));
            break;
        }
        case OpCodes::lmul: {
            auto a = frame.pop_s8();
            auto b = frame.pop_s8();
            frame.push_s8(mul_overflow(a, b));
            break;
        }
        case OpCodes::fmul: {
            auto a = frame.pop_f();
            auto b = frame.pop_f();
            frame.push_f(a * b);
            break;
        }
        case OpCodes::dmul: {
            auto a = frame.pop_d();
            auto b = frame.pop_d();
            frame.push_d(a * b);
            break;
        }
        case OpCodes::idiv: {
            auto divisor = frame.pop_s4();
            auto dividend = frame.pop_s4();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.push_s4(div_overflow(dividend, divisor));
            break;
        }
        case OpCodes::ldiv: {
            auto divisor = frame.pop_s8();
            auto dividend = frame.pop_s8();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.push_s8(div_overflow(dividend, divisor));
            break;
        }
        case OpCodes::fdiv: {
            auto divisor = frame.pop_f();
            auto dividend = frame.pop_f();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.push_f(dividend / divisor);
            break;
        }
        case OpCodes::ddiv: {
            auto divisor = frame.pop_d();
            auto dividend = frame.pop_d();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            frame.push_d(dividend / divisor);
            break;
        }

        case OpCodes::irem: {
            auto divisor = frame.pop_s4();
            auto dividend = frame.pop_s4();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = dividend - mul_overflow(div_overflow(dividend, divisor), divisor);
            frame.push_s4(result);
            break;
        }
        case OpCodes::lrem: {
            auto divisor = frame.pop_s8();
            auto dividend = frame.pop_s8();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = dividend - mul_overflow(div_overflow(dividend, divisor), divisor);
            frame.push_s8(result);
            break;
        }
        case OpCodes::frem: {
            auto divisor = frame.pop_f();
            auto dividend = frame.pop_f();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = std::fmod(dividend, divisor);
            frame.push_f(result);
            break;
        }
        case OpCodes::drem: {
            auto divisor = frame.pop_d();
            auto dividend = frame.pop_d();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            auto result = std::fmod(dividend, divisor);
            frame.push_d(result);
            break;
        }
        case OpCodes::ineg: {
            auto a = frame.pop_s4();
            frame.push_s4(sub_overflow(static_cast<s4>(0), a));
            break;
        }
        case OpCodes::lneg: {
            auto a = frame.pop_s8();
            frame.push_s8(sub_overflow(static_cast<s8>(0), a));
            break;
        }
        case OpCodes::fneg: {
            auto a = frame.pop_f();
            frame.push_f(-a);
            break;
        }
        case OpCodes::dneg: {
            auto a = frame.pop_d();
            frame.push_d(-a);
            break;
        }
        case OpCodes::ishl: {
            auto shift = frame.pop_s4() & 0x1F;
            auto value = frame.pop_s4();
            frame.push_s4(value << shift);
            break;
        }
        case OpCodes::lshl: {
            auto shift = frame.pop_s4() & 0x3F;
            auto value = frame.pop_s8();
            frame.push_s8(value << shift);
            break;
        }
        case OpCodes::ishr: {
            auto shift = frame.pop_s4() & 0x1F;
            auto value = frame.pop_s4();
            frame.push_s4(value >> shift);
            break;
        }
        case OpCodes::lshr: {
            auto shift = frame.pop_s4() & 0x3F;
            auto value = frame.pop_s8();
            frame.push_s8(value >> shift);
            break;
        }
        case OpCodes::iushr: {
            auto shift = frame.pop_s4() & 0x1F;
            auto value = frame.pop_s4();
            // C++20 always performs arithmetic shifts, so the top bits need to be cleared out afterwards
            frame.push_s4((value >> shift) &
                          (future::bit_cast<s4>(std::numeric_limits<u4>::max() >> static_cast<u4>(shift)))
            );
            break;
        }
        case OpCodes::lushr: {
            auto shift = frame.pop_s4() & 0x3F;
            auto value = frame.pop_s8();
            // C++20 always performs arithmetic shifts, so the top bits need to be cleared out afterwards
            frame.push_s8((value >> shift) &
                          (future::bit_cast<s8>(std::numeric_limits<u8>::max() >> static_cast<u8>(shift)))
            );
            break;
        }
        case OpCodes::iand:
            frame.push_s4(frame.pop_s4() & frame.pop_s4());
            break;
        case OpCodes::land:
            frame.push_s8(frame.pop_s8() & frame.pop_s8());
            break;
        case OpCodes::ior:
            frame.push_s4(frame.pop_s4() | frame.pop_s4());
            break;
        case OpCodes::lor:
            frame.push_s8(frame.pop_s8() | frame.pop_s8());
            break;
        case OpCodes::ixor:
            frame.push_s4(frame.pop_s4() ^ frame.pop_s4());
            break;
        case OpCodes::lxor:
            frame.push_s8(frame.pop_s8() ^ frame.pop_s8());
            break;
        case OpCodes::iinc: {
            auto local = code[pc + 1];
            auto value = static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(code[pc + 2])));
            auto result = add_overflow(frame.locals[local].s4, value);
            frame.locals[local] = Value(result);
            return pc + 3;
        }

            /* ======================= Conversions ======================= */
        case OpCodes::i2l:
            frame.push_s8(static_cast<s8>(frame.pop_s4()));
            break;
        case OpCodes::i2f:
            frame.push_f(static_cast<float>(frame.pop_s4()));
            break;
        case OpCodes::i2d:
            frame.push_d(static_cast<double>(frame.pop_s4()));
            break;
        case OpCodes::l2i:
            frame.push_s4(static_cast<s4>(frame.pop_s8()));
            break;
        case OpCodes::l2f:
            frame.push_f(static_cast<float>(frame.pop_s8()));
            break;
        case OpCodes::l2d:
            frame.push_d(static_cast<double>(frame.pop_s8()));
            break;
        case OpCodes::f2i: {
            frame.push_s4(floating_to_integer<float, s4>(frame.pop_f()));
            break;
        }
        case OpCodes::f2l:
            frame.push_s8(floating_to_integer<float, s8>(frame.pop_f()));
            break;
        case OpCodes::f2d:
            frame.push_d(static_cast<double>(frame.pop_f()));
            break;
        case OpCodes::d2i:
            frame.push_s4(floating_to_integer<double, s4>(frame.pop_d()));
            break;
        case OpCodes::d2l:
            frame.push_s8(floating_to_integer<double, s8>(frame.pop_d()));
            break;
        case OpCodes::d2f:
            frame.push_f(static_cast<float>(frame.pop_d()));
            break;
        case OpCodes::i2b:
            frame.push_s4(static_cast<s4>(static_cast<s1>(frame.pop_s4())));
            break;
        case OpCodes::i2c:
            frame.push_s4(static_cast<s4>(static_cast<u2>(frame.pop_s4())));
            break;
        case OpCodes::i2s:
            frame.push_s4(static_cast<s4>(static_cast<s2>(frame.pop_s4())));
            break;

            /* ======================= Comparisons ======================= */
        case OpCodes::lcmp: {
            auto b = frame.pop_s8();
            auto a = frame.pop_s8();
            if (a > b) {
                frame.push_s4(1);
            } else if (a == b) {
                frame.push_s4(0);
            } else {
                frame.push_s4(-1);
            }
            break;
        }
        case OpCodes::fcmpl:
        case OpCodes::fcmpg: {
            // TODO "value set conversion" ?
            auto b = frame.pop_f();
            auto a = frame.pop_f();
            if (a > b) {
                frame.push_s4(1);
            } else if (a == b) {
                frame.push_s4(0);
            } else if (a < b) {
                frame.push_s4(-1);
            } else {
                // at least one of a' or b' is NaN
                frame.push_s4(static_cast<OpCodes>(opcode) == OpCodes::fcmpg ? -1 : 1);
            }
            break;
        }
        case OpCodes::dcmpl:
        case OpCodes::dcmpg: {
            // TODO "value set conversion" ?
            auto b = frame.pop_d();
            auto a = frame.pop_d();
            if (a > b) {
                frame.push_s4(1);
            } else if (a == b) {
                frame.push_s4(0);
            } else if (a < b) {
                frame.push_s4(-1);
            } else {
                // at least one of a' or b' is NaN
                frame.push_s4(static_cast<OpCodes>(opcode) == OpCodes::dcmpl ? -1 : 1);
            }
            break;
        }
        case OpCodes::ifeq:
            return execute_comparison(code, pc, frame.pop_s4() == 0);
        case OpCodes::ifne:
            return execute_comparison(code, pc, frame.pop_s4() != 0);
        case OpCodes::iflt:
            return execute_comparison(code, pc, frame.pop_s4() < 0);
        case OpCodes::ifge:
            return execute_comparison(code, pc, frame.pop_s4() >= 0);
        case OpCodes::ifgt:
            return execute_comparison(code, pc, frame.pop_s4() > 0);
        case OpCodes::ifle:
            return execute_comparison(code, pc, frame.pop_s4() <= 0);
        case OpCodes::if_icmpeq:
            return execute_comparison(code, pc, frame.pop_s4() == frame.pop_s4());
        case OpCodes::if_icmpne:
            return execute_comparison(code, pc, frame.pop_s4() != frame.pop_s4());
        case OpCodes::if_icmplt:
            return execute_comparison(code, pc, frame.pop_s4() > frame.pop_s4());
        case OpCodes::if_icmpge:
            return execute_comparison(code, pc, frame.pop_s4() <= frame.pop_s4());
        case OpCodes::if_icmpgt:
            return execute_comparison(code, pc, frame.pop_s4() < frame.pop_s4());
        case OpCodes::if_icmple:
            return execute_comparison(code, pc, frame.pop_s4() >= frame.pop_s4());
        case OpCodes::if_acmpeq:
            return execute_comparison(code, pc, frame.pop_a() == frame.pop_a());
        case OpCodes::if_acmpne:
            return execute_comparison(code, pc, frame.pop_a() != frame.pop_a());

            /* ======================= Control =======================*/
        case OpCodes::goto_:
            return goto_(code, pc);
        case OpCodes::jsr:
        case OpCodes::ret:
            throw std::runtime_error("jsr and ret are unsupported");
        case OpCodes::tableswitch: {
            size_t opcode_address = pc;

            // skip 0-3 bytes of padding
            pc = (pc + 3) & -4ul;

            s4 default_ = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 low = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 high = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            assert(low <= high);

//            s4 count  = high - low + 1;
            s4 index = frame.pop_s4();

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
            pc = (pc + 3) & -4ul;

            s4 default_ = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 npairs = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            assert(npairs >= 0);

            s4 key = frame.pop_s4();
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
            if (static_cast<OpCodes>(opcode) == OpCodes::lreturn || static_cast<OpCodes>(opcode) == OpCodes::dreturn) {
                frame.locals[0] = frame.pop2();
            } else {
                frame.locals[0] = frame.pop();
            }
            // fallthrough
        case OpCodes::return_: {
            if (thread.stack.parent_frames.empty()) {
                shouldExit = true;
            } else {
                pop_frame(thread, frame);
                return frame.pc + frame.invoke_length;
            }
            break;
        }


            /* ======================= References ======================= */
        case OpCodes::getstatic:
        case OpCodes::putstatic:
        case OpCodes::getfield:
        case OpCodes::putfield: {
            u2 index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto field = frame.clazz->constant_pool.get<CONSTANT_Fieldref_info>(index);

            if (!field.resolved) {
                switch (resolve_class(class_files, field.class_, thread, frame)) {
                    case ClassResolution::OK:
                        break;
                    case ClassResolution::PUSHED_INITIALIZER:
                        return 0;
                    case ClassResolution::NOT_FOUND:
                        throw std::runtime_error("class not found: '" + field.class_->name->value + "'");
                }

                if (!resolve_field_recursive(field.class_->clazz, &field))
                    throw std::runtime_error(
                            "field not found: " + field.class_->name->value + "." + field.name_and_type->name->value +
                            " " + field.name_and_type->descriptor->value);
                assert(field.resolved);
            }

            switch (static_cast<OpCodes>(opcode)) {
                case OpCodes::getstatic: {
                    if (!field.is_static)
                        throw std::runtime_error("field is not static");
                    auto value = field.class_->clazz->static_field_values[field.index];
                    if (field.category == ValueCategory::C1) {
                        frame.push(value);
                    } else {
                        frame.push2(value);
                    }
                    break;
                }
                case OpCodes::putstatic: {
                    if (!field.is_static)
                        throw std::runtime_error("field is not static");
                    Value value;
                    if (field.category == ValueCategory::C1) {
                        value = frame.pop();
                        if (field.is_boolean)
                            value.s4 = value.s4 & 1;
                    } else {
                        value = frame.pop2();
                    }
                    field.class_->clazz->static_field_values[field.index] = value;
                    break;
                }
                case OpCodes::getfield: {
                    if (field.is_static)
                        throw std::runtime_error("field is static");
                    auto objectref = frame.pop_a();
                    if (objectref == JAVA_NULL) {
                        throw std::runtime_error("TODO NullPointerException");
                    }
                    auto value = objectref.data<Value>()[field.index];
                    if (field.category == ValueCategory::C1) {
                        frame.push(value);
                    } else {
                        frame.push2(value);
                    }
                    break;
                }
                case OpCodes::putfield: {
                    if (field.is_static)
                        throw std::runtime_error("field is static");
                    Value value;
                    if (field.category == ValueCategory::C1) {
                        value = frame.pop();
                        if (field.is_boolean)
                            value.s4 = value.s4 & 1;
                    } else {
                        value = frame.pop2();
                    }
                    auto objectref = frame.pop_a();
                    if (objectref == JAVA_NULL) {
                        throw std::runtime_error("TODO NullPointerException");
                    }
                    objectref.data<Value>()[field.index] = value;
                    break;
                }
                default:
                    assert(false);
            }

            return pc + 3;
        }
        case OpCodes::invokespecial: {
            u2 index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            (void) index;
            frame.pop_a(); // this arguemnt for constructor
            // TODO implement invokespecial
            // frame.invoke_length = 3;
            return pc + 3;
        }
        case OpCodes::invokestatic: {
            u2 method_index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto method_ref = std::get<CONSTANT_Methodref_info>(frame.clazz->constant_pool.table[method_index].variant);

            // TODO this is hardcoded for now
            if (method_ref.class_->name->value == "java/lang/System" &&
                method_ref.name_and_type->name->value == "exit" &&
                method_ref.name_and_type->descriptor->value == "(I)V") {
                shouldExit = true;
                return pc;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(I)V") {
                std::cout << frame.pop_s4() << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(J)V") {
                std::cout << frame.pop_s8() << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(F)V") {
                // floatToIntBits
                float f = frame.pop_f();
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
                double d = frame.pop_d();
                s8 i = future::bit_cast<s8>(d);
                if (std::isnan(d)) {
                    // canonical NaN
                    i = future::bit_cast<s8>(0x7ff8000000000000L);
                }
                std::cout << i << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(C)V") {
                std::cout << frame.pop_s4() << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(S)V") {
                std::cout << frame.pop_s4() << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(B)V") {
                std::cout << frame.pop_s4() << "\n";
                return pc + 3;
            } else if (method_ref.name_and_type->name->value == "println" &&
                       method_ref.name_and_type->descriptor->value == "(Z)V") {
                auto value = frame.pop_s4();
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
                switch (resolve_class(class_files, method_ref.class_, thread, frame)) {
                    case ClassResolution::OK:
                        break;
                    case ClassResolution::PUSHED_INITIALIZER:
                        return 0;
                    case ClassResolution::NOT_FOUND:
                        // TODO this prints "A not found" if A was found but a superclass/interface wasn't
                        throw std::runtime_error("class not found: '" + method_ref.class_->name->value + "'");
                }

                clazz = method_ref.class_->clazz;

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
                size_t operand_stack_top = frame.first_operand_index + frame.operands_top;
                frame.operands_top += -method->stack_slots_for_parameters + method->return_size;

                frame.invoke_length = 3;
                thread.stack.parent_frames.push_back(frame);

                frame = {thread.stack, clazz, method, operand_stack_top};
                if (thread.stack.memory_used > thread.stack.memory.size())
                    throw std::runtime_error("stack overflow");
                return 0; // will be set on the new frame
            }
        }
        case OpCodes::new_: {
            u2 index = static_cast<u2>((code[pc + 1]) << 8 | (code[pc + 2]));
            auto &class_info = frame.clazz->constant_pool.get<CONSTANT_Class_info>(index);

            switch (resolve_class(class_files, &class_info, thread, frame)) {
                case ClassResolution::OK:
                    break;
                case ClassResolution::PUSHED_INITIALIZER:
                    return 0;
                case ClassResolution::NOT_FOUND:
                    throw std::runtime_error("class not found: '" + class_info.name->value + "'");
            }
            auto clazz = class_info.clazz;

            auto reference = heap.new_instance(clazz);
            frame.push_a(reference);

            // TODO: the next two instructions are probably dup+invokespecial. We could optimize for that pattern.
            return pc + 3;
        }
        case OpCodes::newarray: {
            s4 count = frame.pop_s4();
            if (count < 0) {
                throw std::runtime_error("TODO NegativeArraySizeException");
            }
            s4 type = code[pc + 1];

            // TODO actual array class
            switch (static_cast<ArrayPrimitiveTypes>(type)) {
                case ArrayPrimitiveTypes::T_INT: {
                    auto reference = heap.new_array<s4>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_BOOLEAN: {
                    // TODO packed? differentiate from byte in load/store via class
                    auto reference = heap.new_array<s1>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_CHAR: {
                    auto reference = heap.new_array<u2>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_FLOAT: {
                    auto reference = heap.new_array<float>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_DOUBLE: {
                    auto reference = heap.new_array<double>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_BYTE: {
                    auto reference = heap.new_array<s1>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_SHORT: {
                    auto reference = heap.new_array<s4>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_LONG: {
                    auto reference = heap.new_array<s8>(nullptr, count);
                    frame.push_a(reference);
                    break;
                }
            }
            return pc + 2;
        }
        case OpCodes::anewarray: {
            s4 count = frame.pop_s4();
            if (count < 0) {
                throw std::runtime_error("TODO NegativeArraySizeException");
            }

            u2 index = static_cast<u2>((code[pc + 1]) << 8 | (code[pc + 2]));
            auto &class_info = frame.clazz->constant_pool.get<CONSTANT_Class_info>(index);
            // TODO actual array class
            (void) class_info;

            auto reference = heap.new_array<Reference>(nullptr, count);
            frame.push_a(reference);
            return pc + 3;
        }
        case OpCodes::arraylength: {
            auto arrayref = frame.pop_a();
            if (arrayref == JAVA_NULL) {
                throw std::runtime_error("TODO NullPointerException");
            }
            frame.push_s4(arrayref.object()->length);
            break;
        }

        case OpCodes::athrow: {
            auto value = frame.pop_a();
            if (value == JAVA_NULL) {
                // TODO If objectref is null, athrow throws a NullPointerException instead of objectref.
                throw std::runtime_error("TODO NullPointerException");
            }
            return handle_throw(thread, frame, shouldExit, pc, value);
        }

        case OpCodes::wide: {
            u2 index = static_cast<u2>((code[pc + 2] << 8) | code[pc + 3]);
            auto &local = frame.locals[index];

            switch (static_cast<OpCodes>(code[pc + 1])) {
                case OpCodes::iload:
                    frame.push_s4(local.s4);
                    break;
                case OpCodes::lload:
                    frame.push_s8(local.s8);
                    break;
                case OpCodes::fload:
                    frame.push_f(local.float_);
                    break;
                case OpCodes::dload:
                    frame.push_d(local.double_);
                    break;
                case OpCodes::aload:
                    frame.push_a(local.reference);
                    break;

                case OpCodes::istore:
                    local = Value(frame.pop_s4());
                    break;
                case OpCodes::lstore:
                    local = Value(frame.pop_s8());
                    break;
                case OpCodes::fstore:
                    local = Value(frame.pop_f());
                    break;
                case OpCodes::dstore:
                    local = Value(frame.pop_d());
                    break;
                case OpCodes::astore:
                    local = Value(frame.pop_a());
                    break;

                case OpCodes::iinc: {
                    s4 constant = future::bit_cast<s2>(static_cast<u2>((code[pc + 4] << 8) | code[pc + 5]));
                    local.s4 += constant;
                    return pc + 6;
                }

                case OpCodes::ret:
                    throw std::runtime_error("jsr and ret are unsupported");
                default:
                    abort();
            }
            return pc + 4;
        }

        case OpCodes::ifnull:
            return execute_comparison(code, pc, frame.pop_a() == JAVA_NULL);
        case OpCodes::ifnonnull:
            return execute_comparison(code, pc, frame.pop_a() != JAVA_NULL);
        case OpCodes::goto_w: {
            s4 offset = static_cast<s4>((code[pc + 1] << 24) | (code[pc + 2] << 16) | (code[pc + 3] << 8) |
                                        code[pc + 4]);
            return pc + static_cast<size_t>(static_cast<ssize_t>(offset));
        }
        case OpCodes::jsr_w:
            throw std::runtime_error("jsr and ret are unsupported");

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

static size_t handle_throw(Thread &thread, Frame &frame, bool &shouldExit, size_t &pc, Reference exception) {
    auto obj = exception.object();
    // TODO this is actually wrong, the stack should be generated when the Throwable is constructed
    std::vector<Frame> stack_trace;

    for (;;) {
        stack_trace.push_back(frame);
        auto &exception_table = frame.method->code_attribute->exception_table;

        auto handler_iter = std::find_if(exception_table.begin(),
                                         exception_table.end(),
                                         [pc, &frame, obj](const ExceptionTableEntry &e) {
                                             if (e.start_pc <= pc && pc < e.end_pc) {
                                                 if (e.catch_type == 0) {
                                                     // "any"
                                                     return true;
                                                 } else {
                                                     // TODO Don't compare by name but resolve the class instead,
                                                     // but without running the class initializer.
                                                     auto &clazz_name = frame.clazz->constant_pool.get<CONSTANT_Class_info>(
                                                             e.catch_type).name->value;
                                                     for (ClassFile *c = obj->clazz;; c = c->super_class->clazz) {
                                                         if (c->this_class->name->value == clazz_name) { return true; }
                                                         if (c->super_class == nullptr) { break; }
                                                     }
                                                     return false;
                                                 }
                                             }
                                             return false;
                                         }
        );

        if (handler_iter == std::end(exception_table)) {
            if (thread.stack.parent_frames.empty()) {
                // Bubbled to the top, no exception handler was found, so exit thread

                std::string message = "Exception in thread \"main\" ";
                message.append(obj->clazz->this_class->name->value);
                message.append("\n");
                for (const Frame &f : stack_trace) {
                    message.append("\tat ");
                    message.append(f.clazz->this_class->name->value);
                    message.append(".");
                    message.append(f.method->name_index->value);

                    auto source_file = std::find_if(f.clazz->attributes.begin(), f.clazz->attributes.end(),
                                                    [](const attribute_info &a) {
                                                        return std::holds_alternative<SourceFile_attribute>(a.variant);
                                                    });
                    // TODO there can be multiple LNT attributes
                    auto line_numbers_iter = std::find_if(f.method->code_attribute->attributes.begin(),
                                                          f.method->code_attribute->attributes.end(),
                                                          [](const attribute_info &a) {
                                                              return std::holds_alternative<LineNumberTable_attribute>(
                                                                      a.variant);
                                                          });
                    message.append("(");
                    if (source_file != std::end(f.clazz->attributes) &&
                        line_numbers_iter != std::end(f.method->code_attribute->attributes)) {
                        auto &line_number_table = std::get<LineNumberTable_attribute>(
                                line_numbers_iter->variant).line_number_table;
                        message.append(std::get<SourceFile_attribute>(source_file->variant).sourcefile_index->value);
                        message.append(":");
                        // NOLINTNEXTLINE
                        for (auto entry = line_number_table.rbegin(); entry != line_number_table.rend(); ++entry) {
                            if (entry->start_pc <= f.pc) {
                                message.append(std::to_string(entry->line_number));
                                break;
                            }
                        }
                    } else {
                        message.append(std::to_string(f.pc));
                    }
                    message.append(")\n");
                }
                std::cerr << message;

                frame.clear();
                frame.push_s4(1);
                shouldExit = true;
                return pc;
            } else {
                pc = pop_frame(thread, frame);
                frame.clear();
                continue;
            }
        } else {
            // Push exception back on stack, continue normal execution.
            frame.clear();
            frame.push_a(exception);
            return handler_iter->handler_pc;
        }
    }
}

static inline size_t pop_frame(Thread &thread, Frame &frame) {
    thread.stack.memory_used = frame.previous_stack_memory_usage;
    frame = thread.stack.parent_frames.back();
    thread.stack.parent_frames.pop_back();

    return frame.pc;
}

Frame::Frame(Stack &stack, ClassFile *clazz, method_info *method, size_t operand_stack_top)
        : clazz(clazz),
          method(method),
          code(&method->code_attribute->code),
          operands_top(0),
          previous_stack_memory_usage(stack.memory_used),
          pc(0),
          invoke_length(0) {
    //assert(method->code_attribute->max_locals >= method->parameter_count);
    assert(stack.memory_used >= method->stack_slots_for_parameters);
    assert(operand_stack_top >= method->stack_slots_for_parameters);

    size_t first_local_index = operand_stack_top - method->stack_slots_for_parameters;
    first_operand_index = first_local_index + method->code_attribute->max_locals;
    stack.memory_used = first_operand_index + method->code_attribute->max_stack;

    locals = {&stack.memory[first_local_index], method->code_attribute->max_locals};
    operands = {&stack.memory[first_operand_index], method->code_attribute->max_stack};

    // TODO think about value set conversion
    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.8.3
}

template<typename Element>
size_t array_store(Frame &frame, size_t pc) {
    auto value = frame.pop<Element>();
    auto index = frame.pop_s4();

    auto arrayref = frame.pop_a();
    if (arrayref == JAVA_NULL) {
        throw std::runtime_error("TODO NullPointerException");
    }

    if (index < 0 || index >= arrayref.object()->length) {
        throw std::runtime_error("TODO ArrayIndexOutOfBoundsException");
    }

    arrayref.data<Element>()[index] = value;
    return pc + 1;
}

template<typename Element>
size_t array_load(Frame &frame, size_t pc) {
    auto index = frame.pop_s4();

    auto arrayref = frame.pop_a();
    if (arrayref == JAVA_NULL) {
        throw std::runtime_error("TODO NullPointerException");
    }

    if (index < 0 || index >= arrayref.object()->length) {
        throw std::runtime_error("TODO ArrayIndexOutOfBoundsException");
    }

    frame.push<Element>(arrayref.data<Element>()[index]);
    return pc + 1;
}
