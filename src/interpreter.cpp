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
        /* ================ Constants ================ */
//        case OpCodes::nop:
//            break;
//        case OpCodes::aconst_null:
//            TODO what is NULL?
//            break;
        case OpCodes::iconst_m1:
        case OpCodes::iconst_0:
        case OpCodes::iconst_1:
        case OpCodes::iconst_2:
        case OpCodes::iconst_3:
        case OpCodes::iconst_4:
        case OpCodes::iconst_5:
            frame.stack.push(code[pc] - static_cast<u1>(OpCodes::iconst_0));
            break;

        case OpCodes::lconst_0:
        case OpCodes::lconst_1:
            frame.stack.push(code[pc] - static_cast<u1>(OpCodes::lconst_0));
            break;
        case OpCodes::fconst_0:
        case OpCodes::fconst_1:
        case OpCodes::fconst_2:
            frame.stack.push(code[pc] - static_cast<u1>(OpCodes::fconst_0));
            break;
        case OpCodes::dconst_0:
        case OpCodes::dconst_1:
            frame.stack.push(code[pc] - static_cast<u1>(OpCodes::dconst_0));
            break;
        case OpCodes::bipush: {
            auto value = static_cast<s4>(code[pc + 1]);
            frame.stack.push(value);
            return pc + 2;
        }
//        case OpCodes::sipush: {
//            auto value = static_cast<u2>(code[pc + 1] << 8) | static_cast<u2>(code[pc + 2]);
//            frame.stack.push(value);
//            return pc + 2;
//        }
// TODO these read from the runtime constant pool
//        case OpCodes::ldc:
//        case OpCodes::ldc_w:
//        case OpCodes::ldc2_w:
//          break;


            /* ================ Loads ================ */
//        case OpCodes::iload: {
//            auto local = code[pc + 1];
//            frame.stack.push(frame.locals[local]);
//            break;
//        }
//        case OpCodes::lload: {
//            auto index = code[pc + 1];
//            auto high = frame.locals[index];
//            auto low = frame.locals[index + 1];
//            frame.stack.push((high << 8) | low);
//            break;
//        }
//        case OpCodes::fload:
//        case OpCodes::dload:
//        case OpCodes::aload:
        case OpCodes::iload_0:
        case OpCodes::iload_1:
        case OpCodes::iload_2:
        case OpCodes::iload_3:
            frame.stack.push(frame.locals[code[pc] - static_cast<u1>(OpCodes::iload_0)]);
            break;
        case OpCodes::istore_0:
        case OpCodes::istore_1:
        case OpCodes::istore_2:
        case OpCodes::istore_3:
            frame.locals[code[pc] - static_cast<u1>(OpCodes::istore_0)] = frame.stack.top();
            frame.stack.pop();
            break;


        case OpCodes::iinc: {
            auto local = code[pc + 1];
            auto value = static_cast<s1>(code[pc + 2]);
            // TODO ???
            frame.locals[local] += value;
            return pc + 3;
        }
        case OpCodes::imul: {
            auto a = static_cast<s4>(frame.stack.top());
            frame.stack.pop();
            auto b = static_cast<s4>(frame.stack.top());
            frame.stack.pop();
            frame.stack.push(a * b);
            break;
        }
        case OpCodes::idiv: {
            auto divisor = static_cast<s4>(frame.stack.top());
            frame.stack.pop();
            auto dividend = static_cast<s4>(frame.stack.top());
            frame.stack.pop();
            if (divisor == 0) {
                // TODO ArithmeticException
                throw std::runtime_error("Division by 0");
            }
            // TODO test rounding
            frame.stack.push(dividend / divisor);
            break;
        }

            // methods
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

Frame::Frame(const ClassFile &clas, size_t locals_count, std::unique_ptr<Frame> previous_frame)
        : clas(clas), previous_frame(std::move(previous_frame)) {
    this->locals.resize(locals_count);
}