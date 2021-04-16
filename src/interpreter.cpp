#include "interpreter.hpp"

#include <algorithm>
#include <string>
#include <iostream>
#include "opcodes.hpp"

static const u2 MAIN_ACCESS_FLAGS = (static_cast<u2>(FieldInfoAccessFlags::ACC_PUBLIC) |
                                     static_cast<u2>(FieldInfoAccessFlags::ACC_STATIC));
static const auto MAIN_NAME = "main";
static const auto MAIN_DESCRIPTOR = "([Ljava/lang/String;)V";

static ssize_t execute_instruction(Frame &frame, const std::vector<u1> &code, ssize_t pc);

static ssize_t execute_comparison(const std::vector<u1> &code, ssize_t pc, bool condition);

static ssize_t goto_(const std::vector<u1> &code, ssize_t pc);

int interpret(const std::vector<ClassFile> &class_files, ssize_t main_class_index) {
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
    std::unique_ptr<Frame> p(new Frame(main, code.max_locals, nullptr));
    stack.current_frame = std::move(p);

    ssize_t pc = 0;
    do {
        pc = execute_instruction(*stack.current_frame, code.code, pc);
    } while (pc >= 0);

    // print exit code
    if (stack.current_frame->stack.empty()) {
        return 0;
    } else {
        return static_cast<int>(stack.current_frame->stack.top());
    }
}

static ssize_t execute_instruction(Frame &frame, const std::vector<u1> &code, ssize_t pc) {
    auto opcode = static_cast<OpCodes>(code[pc]);
    switch (opcode) {
        /* ======================= Constants ======================= */
        case OpCodes::nop:
            break;
//        case OpCodes::aconst_null:
//            // TODO what is NULL?
//            frame.stack_push(0);
//            break;
        case OpCodes::iconst_m1:
        case OpCodes::iconst_0:
        case OpCodes::iconst_1:
        case OpCodes::iconst_2:
        case OpCodes::iconst_3:
        case OpCodes::iconst_4:
        case OpCodes::iconst_5:
            frame.stack_push(code[pc] - static_cast<u1>(OpCodes::iconst_0));
            break;

        case OpCodes::lconst_0:
        case OpCodes::lconst_1:
            frame.stack_push(code[pc] - static_cast<u1>(OpCodes::lconst_0));
            break;
        case OpCodes::fconst_0:
        case OpCodes::fconst_1:
        case OpCodes::fconst_2:
            frame.stack_push(code[pc] - static_cast<u1>(OpCodes::fconst_0));
            break;
        case OpCodes::dconst_0:
        case OpCodes::dconst_1:
            frame.stack_push(code[pc] - static_cast<u1>(OpCodes::dconst_0));
            break;
        case OpCodes::bipush: {
            auto value = static_cast<s4>(code[pc + 1]);
            frame.stack_push(value);
            return pc + 2;
        }
//        case OpCodes::sipush: {
//            auto value = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
//            frame.stack_push(value);
//            return pc + 2;
//        }
// TODO the following actually read from the runtime constant pool
        case OpCodes::ldc: {
            auto index = code[pc + 1];
            auto &entry = frame.clas.constant_pool.table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.stack_push(i->bytes);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.stack_push(f->value);
            } else {
                throw std::runtime_error("ldc refers to invalid/unimplemented type");
            }
            return pc + 2;
        }
//        case OpCodes::ldc_w:
        case OpCodes::ldc2_w: {
            auto index = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
            auto &entry = frame.clas.constant_pool.table[index];
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
//        case OpCodes::iload: {
        case OpCodes::fload: {
            auto local = code[pc + 1];
            frame.stack_push(frame.locals[local]);
            break;
        }
        case OpCodes::lload: {
            auto index = code[pc + 1];
            auto high = frame.locals[index];
            auto low = frame.locals[index + 1];
            frame.stack_push((high << 8) | low);
            break;
        }
//        case OpCodes::dload:
//        case OpCodes::aload:
        case OpCodes::iload_0:
        case OpCodes::iload_1:
        case OpCodes::iload_2:
        case OpCodes::iload_3:
            frame.stack_push(frame.locals[code[pc] - static_cast<u1>(OpCodes::iload_0)]);
            break;
        case OpCodes::lload_0:
        case OpCodes::lload_1:
        case OpCodes::lload_2:
        case OpCodes::lload_3: {
            auto index = code[pc] - static_cast<u1>(OpCodes::lload_0);
            auto high = frame.locals[index];
            auto low = frame.locals[index + 1];
            frame.stack_push((high << 8) | low);
            break;
        }

            /* ======================= Stores ======================= */
        case OpCodes::istore:
            frame.locals[code[pc + 1]] = frame.stack_pop();
            return pc + 2;
        case OpCodes::lstore: {
            auto value = frame.stack_pop();
            frame.locals[code[pc + 1]] = (value >> 8) & 0xff;
            frame.locals[code[pc + 1] + 1] = value & 0xff;
            return pc + 2;
        }
        case OpCodes::istore_0:
        case OpCodes::istore_1:
        case OpCodes::istore_2:
        case OpCodes::istore_3:
            frame.locals[code[pc] - static_cast<u1>(OpCodes::istore_0)] = frame.stack_pop();
            break;
        case OpCodes::lstore_0:
        case OpCodes::lstore_1:
        case OpCodes::lstore_2:
        case OpCodes::lstore_3: {
            auto value = frame.stack_pop();
            frame.locals[code[pc] - static_cast<u1>(OpCodes::lstore_0)] = (value >> 8) & 0xff;
            frame.locals[code[pc] - static_cast<u1>(OpCodes::lstore_0) + 1] = value & 0xff;
            break;
        }

            /* ======================= Math =======================*/
        case OpCodes::iadd: {
            auto a = static_cast<s4>(frame.stack_pop());
            auto b = static_cast<s4>(frame.stack_pop());
            // TODO make sure overflow occurs
            frame.stack_push(a + b);
            break;
        }
        case OpCodes::ladd: {
            auto a = frame.stack_pop();
            auto b = frame.stack_pop();
            frame.stack_push(a + b);
            break;
        }
        case OpCodes::isub: {
            auto b = static_cast<s4>(frame.stack_pop());
            auto a = static_cast<s4>(frame.stack_pop());
            frame.stack_push(a - b);
            break;
        }
        case OpCodes::imul: {
            auto a = static_cast<s4>(frame.stack_pop());
            auto b = static_cast<s4>(frame.stack_pop());
            // TODO make sure overflow occurs
            frame.stack_push(a * b);
            break;
        }
        case OpCodes::lmul: {
            auto a = static_cast<s4>(frame.stack_pop());
            auto b = static_cast<s4>(frame.stack_pop());
            frame.stack_push(a * b);
            break;
        }

        case OpCodes::iinc: {
            auto local = code[pc + 1];
            auto value_u = code[pc + 2];
            auto value = reinterpret_cast<s1 &>(value_u);
            frame.locals[local] += value;
            return pc + 3;
        }

        case OpCodes::idiv: {
            // TODO ensure overflow/...
            auto divisor = static_cast<s4>(frame.stack_pop());
            auto dividend = static_cast<s4>(frame.stack_pop());
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            // TODO test rounding
            frame.stack_push(dividend / divisor);
            break;
        }
        case OpCodes::ldiv: {
            auto divisor = static_cast<s4>(frame.stack_pop());
            auto dividend = static_cast<s4>(frame.stack_pop());
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            // TODO test rounding
            frame.stack_push(dividend / divisor);
            break;
        }

            /* ======================= Conversions ======================= */
        case OpCodes::l2i: {
            auto value = static_cast<s4>(frame.stack_pop());
            frame.stack_push(value);
            break;
        }

            /* ======================= Comparisons ======================= */
        case OpCodes::lcmp: {
            auto b_u = frame.stack_pop();
            auto a_u = frame.stack_pop();
            const auto b = reinterpret_cast<s8 &>(b_u);
            const auto a = reinterpret_cast<s8 &>(a_u);
            frame.stack_push(std::clamp(a - b, -1LL, 1LL));
            break;
        }
        case OpCodes::ifeq:
            return execute_comparison(code, pc, frame.stack_pop() == 0);
        case OpCodes::ifne:
            return execute_comparison(code, pc, frame.stack_pop() != 0);
        case OpCodes::iflt:
            return execute_comparison(code, pc, frame.stack_pop() < 0);
        case OpCodes::ifge:
            return execute_comparison(code, pc, frame.stack_pop() >= 0);
        case OpCodes::ifgt:
            return execute_comparison(code, pc, frame.stack_pop() > 0);
        case OpCodes::ifle:
            return execute_comparison(code, pc, frame.stack_pop() <= 0);
        case OpCodes::if_acmpeq:
        case OpCodes::if_icmpeq:
            return execute_comparison(code, pc, frame.stack_pop() == frame.stack_pop());
        case OpCodes::if_acmpne:
        case OpCodes::if_icmpne:
            return execute_comparison(code, pc, frame.stack_pop() != frame.stack_pop());
        case OpCodes::if_icmplt:
            return execute_comparison(code, pc, frame.stack_pop() > frame.stack_pop());
        case OpCodes::if_icmpge:
            return execute_comparison(code, pc, frame.stack_pop() <= frame.stack_pop());
        case OpCodes::if_icmpgt:
            return execute_comparison(code, pc, frame.stack_pop() < frame.stack_pop());
        case OpCodes::if_icmple:
            return execute_comparison(code, pc, frame.stack_pop() >= frame.stack_pop());

            /* ======================= Control =======================*/
        case OpCodes::goto_:
            return goto_(code, pc);
        case OpCodes::return_:
            return -1;


            /* ======================= References ======================= */
        case OpCodes::invokestatic: {
            auto method_index = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
            // TODO this is really an index into the run-time constant pool
            auto method = std::get<CONSTANT_Methodref_info>(frame.clas.constant_pool.table[method_index].variant);

            // TODO this is hardcoded for now
            if (method.class_->name->value == "java/lang/System" && method.name_and_type->name->value == "exit" &&
                method.name_and_type->descriptor->value == "(I)V") {
                return -1;
            } else {
                throw std::runtime_error("Unimplemented invokestatic");
            }
        }

        default:
            throw std::runtime_error(
                    "Unimplemented/unknown opcode " + std::to_string(code[pc]) + " at " + std::to_string(pc)
            );
    }

    return pc + 1;
}

static ssize_t execute_comparison(const std::vector<u1> &code, ssize_t pc, bool condition) {
    if (condition) {
        return goto_(code, pc);
    } else {
        return pc + 3;
    }
}

static ssize_t goto_(const std::vector<u1> &code, ssize_t pc) {
    u2 offset_u = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
    const s2 offset = reinterpret_cast<s2 &>(offset_u);
    // TODO test negative offset
    return pc + offset;
}


Frame::Frame(const ClassFile &clas, size_t locals_count, std::unique_ptr<Frame> previous_frame)
        : clas(clas), previous_frame(std::move(previous_frame)) {
    this->locals.resize(locals_count);
}

u8 Frame::stack_pop() {
    auto v = static_cast<s4>(this->stack.top());
    this->stack.pop();
    return v;
}

void Frame::stack_push(u8 v) {
    this->stack.push(v);
}