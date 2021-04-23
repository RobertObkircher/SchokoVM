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

static size_t execute_instruction(Thread &thread, Frame &frame, size_t pc,
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
        frame.pc = execute_instruction(thread, frame, frame.pc, class_files, shouldExit);
    }

    // print exit code
    if (frame.operands_count == 0) {
        return 0;
    } else {
        return frame.stack_pop().s4;
    }
}

static inline size_t execute_instruction(Thread &thread, Frame &frame, size_t pc,
                                         std::unordered_map<std::string_view, ClassFile *> &class_files,
                                         bool &shouldExit) {
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
//            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
//                frame.stack_push(f->value);
            } else {
                throw std::runtime_error("ldc refers to invalid/unimplemented type");
            }
            return pc + 2;
        }
//        case OpCodes::ldc_w:
        case OpCodes::ldc2_w: {
            size_t index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            auto &entry = frame.clazz->constant_pool.table[index];
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
            frame.stack_push(frame.locals[local].s4);
            return pc + 2;
        }
        case OpCodes::fload: {
            auto local = code[pc + 1];
            frame.stack_push(frame.locals[local].float_);
            return pc + 2;
        }
        case OpCodes::lload: {
            // In a slight derivation from the spec, longs are stored in a single local
            auto index = code[pc + 1];
            frame.stack_push(frame.locals[index].s8);
            return pc + 2;
        }
//        case OpCodes::dload:
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

            /* ======================= Stores ======================= */
        case OpCodes::istore:
            frame.locals[code[pc + 1]] = {frame.stack_pop().s4};
            return pc + 2;
        case OpCodes::lstore: {
            // In a slight derivation from the spec, longs are stored in a single local
            auto value = frame.stack_pop().s8;
            frame.locals[code[pc + 1]] = {value};
            return pc + 2;
        }
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
            frame.locals[local] = {result};
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
            auto value = static_cast<s4>(frame.stack_pop().s8);
            frame.stack_push(value);
            break;
        }

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

        case OpCodes::ireturn:
        case OpCodes::lreturn:
        case OpCodes::freturn:
        case OpCodes::dreturn:
        case OpCodes::areturn:
            frame.locals[0] = frame.stack_pop();
            // fallthrough
        case OpCodes::return_: {
            if (thread.stack.frames.empty()) {
                shouldExit = true;
            } else {
                thread.stack.memory_used = frame.previous_stack_memory_usage;
                frame = thread.stack.frames[thread.stack.frames.size() - 1];
                thread.stack.frames.pop_back();
                return frame.pc;
            }
            break;
        }


            /* ======================= References ======================= */
        case OpCodes::invokestatic: {
            size_t method_index = static_cast<u2>((code[pc + 1] << 8) | code[pc + 2]);
            // TODO this is really an index into the run-time constant pool
            auto method = std::get<CONSTANT_Methodref_info>(frame.clazz->constant_pool.table[method_index].variant);

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
                if (method.class_->clazz == nullptr) {
                    auto result = class_files.find(method.class_->name->value);
                    if (result == class_files.end()) {
                        throw std::runtime_error("class not found: '" + method.class_->name->value + "'");
                    }
                    method.class_->clazz = result->second;

                    // TODO if clazz is not initialized: create a stackframe with the initializer but do not advance the pc of this frame
                }

                ClassFile *clazz = method.class_->clazz;

                auto method_iter = std::find_if(clazz->methods.begin(), clazz->methods.end(),
                                                [method](const method_info &m) {
                                                    return m.name_index->value == method.name_and_type->name->value &&
                                                           m.descriptor_index->value ==
                                                           method.name_and_type->descriptor->value
                                                        // TODO m.access_flags
                                                            ;
                                                });
                if (method_iter == std::end(clazz->methods)) {
                    throw std::runtime_error("Couldn't find method: '" + method.name_and_type->name->value + "' " +
                                             method.name_and_type->descriptor->value + " in class " +
                                             method.class_->name->value);
                }

                method_info *target_method = &*method_iter;

                size_t operand_stack_top = frame.first_operand_index + frame.operands_count;
                frame.operands_count += target_method->minus_args_plus_result;

                frame.pc += 3;
                thread.stack.frames.push_back(frame);

                frame = {thread.stack, clazz, target_method, operand_stack_top};
                return 0;
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
    //assert(method->code_attribute->max_locals >= method->nargs);
    assert(stack.memory_used >= method->nargs);
    assert(operand_stack_top >= method->nargs);

    size_t first_local_index = operand_stack_top - method->nargs;
    first_operand_index = first_local_index + method->code_attribute->max_locals;
    stack.memory_used = first_operand_index + method->code_attribute->max_stack;

    locals = {&stack.memory[first_local_index], method->code_attribute->max_locals};
    operands = {&stack.memory[first_operand_index], method->code_attribute->max_stack};

    // if we need to move at least one argument
    if (method->argument_takes_two_local_variables[255]) {
        size_t target = method->nargs_stack_slots;
        size_t source = method->nargs;
        // we know there is at least one:
        do {
            --target;
            --source;

            if (method->argument_takes_two_local_variables[source]) {
                --target;
                if (source == target) break;
            }
            assert(target > source);
            locals[target] = locals[source];
        } while (source != 0);
    }

    // TODO think about value set conversion
    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.8.3
}
