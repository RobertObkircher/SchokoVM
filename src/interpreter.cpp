#include "interpreter.hpp"

#include <limits>
#include <string>
#include <iostream>
#include <cmath>
#include <cstdlib>

#include "exceptions.hpp"
#include "opcodes.hpp"
#include "future.hpp"
#include "math.hpp"
#include "classloading.hpp"
#include "native.hpp"

// Table 6.5.newarray-A. Array type codes
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

static void execute_instruction(Thread &thread, Frame &frame, bool &should_exit);

static void execute_comparison(Frame &frame, bool condition);

static void goto_(Frame &frame);

static void handle_throw(Thread &thread, Frame &frame, Reference exception, bool &should_exit);

static void pop_frame(Thread &thread, Frame &frame, bool &should_exit);

template<typename Element>
void array_store(Thread &thread, Frame &frame);

template<typename Element>
void array_load(Thread &thread, Frame &frame);

void fill_multi_array(Reference &reference, ClassFile *element_type, const std::span<s4> &counts);

[[nodiscard]] static bool method_resolution(ClassInterface_Methodref &method);

static void native_call(method_info *method, Thread &thread, Frame &frame, bool &should_exit);

Value interpret(Thread &thread, method_info *method) {
    if (thread.current_exception != JAVA_NULL) {
        // TODO is it possible to call a function while an exception is being thrown?
        assert(false);
        return Value();
    }

    [[maybe_unused]] auto frames = thread.stack.frames.size();
    [[maybe_unused]] auto memory_used = thread.stack.memory_used;
    Frame frame{thread.stack, method, thread.stack.memory_used, true};

    if (!method->clazz->is_initialized) {
        if (resolve_class(method->clazz->this_class)) {
            assert(thread.current_exception != JAVA_NULL);
            return Value();
        }
        if (initialize_class(method->clazz, thread, frame)) {
            assert(thread.current_exception != JAVA_NULL);
            return Value();
        }
        assert(thread.current_exception == JAVA_NULL);
    }

    bool should_exit = false;
    while (!should_exit) {
        if (thread.current_exception != JAVA_NULL) {
            handle_throw(thread, frame, thread.current_exception, should_exit);
        } else {
            execute_instruction(thread, frame, should_exit);
        }
    }

    assert(thread.stack.frames.size() == frames);
    assert(thread.stack.memory_used == memory_used);

    return method->return_category == 0 ? Value() : frame.locals[0];
}

static inline void execute_instruction(Thread &thread, Frame &frame, bool &should_exit) {
    std::vector<u1> &code = *frame.code;
    auto opcode = code[frame.pc];
    // TODO implement remaining opcodes. The ones that are currently commented/missing out have no test coverage whatsoever
    switch (static_cast<OpCodes>(opcode)) {
        /* ======================= Constants ======================= */
        case OpCodes::nop:
            break;
        case OpCodes::aconst_null:
            frame.push<Reference>(JAVA_NULL);
            break;
        case OpCodes::iconst_m1:
        case OpCodes::iconst_0:
        case OpCodes::iconst_1:
        case OpCodes::iconst_2:
        case OpCodes::iconst_3:
        case OpCodes::iconst_4:
        case OpCodes::iconst_5:
            frame.push<s4>(opcode - static_cast<u1>(OpCodes::iconst_0));
            break;

        case OpCodes::lconst_0:
        case OpCodes::lconst_1:
            frame.push<s8>(opcode - static_cast<u1>(OpCodes::lconst_0));
            break;
        case OpCodes::fconst_0:
        case OpCodes::fconst_1:
        case OpCodes::fconst_2:
            frame.push<float>(static_cast<float>(opcode - static_cast<u1>(OpCodes::fconst_0)));
            break;
        case OpCodes::dconst_0:
        case OpCodes::dconst_1:
            frame.push<double>(static_cast<double>(opcode - static_cast<u1>(OpCodes::dconst_0)));
            break;
        case OpCodes::bipush: {
            frame.push<s4>(static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(frame.consume_u1()))));
            break;
        }
        case OpCodes::sipush: {
            u2 value = frame.consume_u2();
            frame.push<s4>(future::bit_cast<s2>(value));
            break;
        }
        case OpCodes::ldc: {
            auto index = frame.consume_u1();
            auto &entry = frame.constant_pool->table[index];
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.push<s4>(i->value);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.push<float>(f->value);
            } else if (auto c = std::get_if<CONSTANT_Class_info>(&entry.variant)) {
                if (resolve_class(c)) {
                    return;
                }
                frame.push<Reference>(Reference{c->clazz});
            } else if (auto s = std::get_if<CONSTANT_String_info>(&entry.variant)) {
                frame.push<Reference>(Heap::get().load_string(s->string));
            } else {
                // TODO: "a symbolic reference to a method type, a method handle, or a dynamically-computed constant." (?)
                throw std::runtime_error("ldc refers to invalid/unimplemented type");
            }
            break;
        }
        case OpCodes::ldc_w: {
            size_t index = frame.consume_u2();
            auto &entry = frame.constant_pool->table[index];
            // TODO common function to turn constan pool reference into value
            if (auto i = std::get_if<CONSTANT_Integer_info>(&entry.variant)) {
                frame.push<s4>(i->value);
            } else if (auto f = std::get_if<CONSTANT_Float_info>(&entry.variant)) {
                frame.push<float>(f->value);
            } else if (auto c = std::get_if<CONSTANT_Class_info>(&entry.variant)) {
                if (resolve_class(c)) {
                    return;
                }
                frame.push<Reference>(Reference{c->clazz});
            } else if (auto s = std::get_if<CONSTANT_String_info>(&entry.variant)) {
                frame.push<Reference>(Heap::get().load_string(s->string));
            } else {
                throw std::runtime_error("ldc_w refers to invalid/unimplemented type");
            }
            break;
        }
        case OpCodes::ldc2_w: {
            size_t index = frame.consume_u2();
            auto &entry = frame.constant_pool->table[index];
            if (auto l = std::get_if<CONSTANT_Long_info>(&entry.variant)) {
                frame.push<s8>(l->value);
            } else if (auto d = std::get_if<CONSTANT_Double_info>(&entry.variant)) {
                frame.push<double>(d->value);
            } else {
                throw std::runtime_error("ldc2_w refers to invalid/unimplemented type");
            }
            break;
        }

            /* ======================= Loads ======================= */
        case OpCodes::iload:
            frame.push<s4>(frame.locals[frame.consume_u1()].s4);
            break;
        case OpCodes::lload:
            frame.push<s8>(frame.locals[frame.consume_u1()].s8);
            break;
        case OpCodes::fload:
            frame.push<float>(frame.locals[frame.consume_u1()].float_);
            break;
        case OpCodes::dload:
            frame.push<double>(frame.locals[frame.consume_u1()].double_);
            break;
        case OpCodes::aload:
            frame.push<Reference>(frame.locals[frame.consume_u1()].reference);
            break;

        case OpCodes::iload_0:
        case OpCodes::iload_1:
        case OpCodes::iload_2:
        case OpCodes::iload_3:
            frame.push<s4>(frame.locals[opcode - static_cast<u1>(OpCodes::iload_0)].s4);
            break;
        case OpCodes::lload_0:
        case OpCodes::lload_1:
        case OpCodes::lload_2:
        case OpCodes::lload_3:
            frame.push<s8>(frame.locals[(static_cast<u1>(opcode - static_cast<u1>(OpCodes::lload_0)))].s8);
            break;
        case OpCodes::fload_0:
        case OpCodes::fload_1:
        case OpCodes::fload_2:
        case OpCodes::fload_3:
            frame.push<float>(frame.locals[opcode - static_cast<u1>(OpCodes::fload_0)].float_);
            break;
        case OpCodes::dload_0:
        case OpCodes::dload_1:
        case OpCodes::dload_2:
        case OpCodes::dload_3:
            frame.push<double>(frame.locals[static_cast<u1>(opcode - static_cast<u1>(OpCodes::dload_0))].double_);
            break;
        case OpCodes::aload_0:
        case OpCodes::aload_1:
        case OpCodes::aload_2:
        case OpCodes::aload_3:
            frame.push<Reference>(frame.locals[static_cast<u1>(opcode - static_cast<u1>(OpCodes::aload_0))].reference);
            break;
        case OpCodes::iaload:
            array_load<s4>(thread, frame);
            break;
        case OpCodes::laload:
            array_load<s8>(thread, frame);
            break;
        case OpCodes::faload:
            array_load<float>(thread, frame);
            break;
        case OpCodes::daload:
            array_load<double>(thread, frame);
            break;
        case OpCodes::aaload:
            array_load<Reference>(thread, frame);
            break;
        case OpCodes::baload:
            array_load<s1>(thread, frame);
            break;
        case OpCodes::caload:
            array_load<u2>(thread, frame);
            break;
        case OpCodes::saload:
            array_load<s4>(thread, frame);
            break;

            /* ======================= Stores ======================= */
        case OpCodes::istore:
            frame.locals[frame.consume_u1()] = Value(frame.pop<s4>());
            break;
        case OpCodes::lstore:
            frame.locals[frame.consume_u1()] = Value(frame.pop<s8>());
            break;
        case OpCodes::fstore:
            frame.locals[frame.consume_u1()] = Value(frame.pop<float>());
            break;
        case OpCodes::dstore:
            frame.locals[frame.consume_u1()] = Value(frame.pop<double>());
            break;
        case OpCodes::astore:
            frame.locals[frame.consume_u1()] = Value(frame.pop<Reference>());
            break;
        case OpCodes::istore_0:
        case OpCodes::istore_1:
        case OpCodes::istore_2:
        case OpCodes::istore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::istore_0)] = Value(frame.pop<s4>());
            break;
        case OpCodes::lstore_0:
        case OpCodes::lstore_1:
        case OpCodes::lstore_2:
        case OpCodes::lstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::lstore_0)] = Value(frame.pop<s8>());
            break;
        case OpCodes::fstore_0:
        case OpCodes::fstore_1:
        case OpCodes::fstore_2:
        case OpCodes::fstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::fstore_0)] = Value(frame.pop<float>());
            break;
        case OpCodes::dstore_0:
        case OpCodes::dstore_1:
        case OpCodes::dstore_2:
        case OpCodes::dstore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::dstore_0)] = Value(frame.pop<double>());
            break;
        case OpCodes::astore_0:
        case OpCodes::astore_1:
        case OpCodes::astore_2:
        case OpCodes::astore_3:
            frame.locals[opcode - static_cast<u1>(OpCodes::astore_0)] = Value(frame.pop<Reference>());
            break;
        case OpCodes::iastore:
            array_store<s4>(thread, frame);
            break;
        case OpCodes::lastore:
            array_store<s8>(thread, frame);
            break;
        case OpCodes::fastore:
            array_store<float>(thread, frame);
            break;
        case OpCodes::dastore:
            array_store<double>(thread, frame);
            break;
        case OpCodes::aastore:
            array_store<Reference>(thread, frame);
            break;
        case OpCodes::bastore:
            array_store<s1>(thread, frame);
            break;
        case OpCodes::castore:
            array_store<u2>(thread, frame);
            break;
        case OpCodes::sastore:
            array_store<s4>(thread, frame);
            break;

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
            auto b = frame.pop<s4>();
            auto a = frame.pop<s4>();
            auto result = add_overflow(a, b);
            frame.push<s4>(result);
            break;
        }
        case OpCodes::ladd: {
            auto b = frame.pop<s8>();
            auto a = frame.pop<s8>();
            auto result = add_overflow(a, b);
            frame.push<s8>(result);
            break;
        }
        case OpCodes::fadd: {
            auto b = frame.pop<float>();
            auto a = frame.pop<float>();
            frame.push<float>(a + b);
            break;
        }
        case OpCodes::dadd: {
            auto b = frame.pop<double>();
            auto a = frame.pop<double>();
            frame.push<double>(a + b);
            break;
        }
        case OpCodes::isub: {
            auto b = frame.pop<s4>();
            auto a = frame.pop<s4>();
            auto result = sub_overflow(a, b);
            frame.push<s4>(result);
            break;
        }
        case OpCodes::lsub: {
            s8 b = frame.pop<s8>();
            s8 a = frame.pop<s8>();
            frame.push<s8>(sub_overflow(a, b));
            break;
        }
        case OpCodes::fsub: {
            auto b = frame.pop<float>();
            auto a = frame.pop<float>();
            frame.push<float>(a - b);
            break;
        }
        case OpCodes::dsub: {
            auto b = frame.pop<double>();
            auto a = frame.pop<double>();
            frame.push<double>(a - b);
            break;
        }
        case OpCodes::imul: {
            auto a = frame.pop<s4>();
            auto b = frame.pop<s4>();
            frame.push<s4>(mul_overflow(a, b));
            break;
        }
        case OpCodes::lmul: {
            auto a = frame.pop<s8>();
            auto b = frame.pop<s8>();
            frame.push<s8>(mul_overflow(a, b));
            break;
        }
        case OpCodes::fmul: {
            auto a = frame.pop<float>();
            auto b = frame.pop<float>();
            frame.push<float>(a * b);
            break;
        }
        case OpCodes::dmul: {
            auto a = frame.pop<double>();
            auto b = frame.pop<double>();
            frame.push<double>(a * b);
            break;
        }
        case OpCodes::idiv: {
            auto divisor = frame.pop<s4>();
            auto dividend = frame.pop<s4>();
            if (divisor == 0) {
                return throw_new_ArithmeticException_division_by_zero(thread, frame);
            }
            frame.push<s4>(div_overflow(dividend, divisor));
            break;
        }
        case OpCodes::ldiv: {
            auto divisor = frame.pop<s8>();
            auto dividend = frame.pop<s8>();
            if (divisor == 0) {
                return throw_new_ArithmeticException_division_by_zero(thread, frame);
            }
            frame.push<s8>(div_overflow(dividend, divisor));
            break;
        }
        case OpCodes::fdiv: {
            auto divisor = frame.pop<float>();
            auto dividend = frame.pop<float>();
            frame.push<float>(dividend / divisor);
            break;
        }
        case OpCodes::ddiv: {
            auto divisor = frame.pop<double>();
            auto dividend = frame.pop<double>();
            frame.push<double>(dividend / divisor);
            break;
        }

        case OpCodes::irem: {
            auto divisor = frame.pop<s4>();
            auto dividend = frame.pop<s4>();
            if (divisor == 0) {
                return throw_new_ArithmeticException_division_by_zero(thread, frame);
            }
            auto result = dividend - mul_overflow(div_overflow(dividend, divisor), divisor);
            frame.push<s4>(result);
            break;
        }
        case OpCodes::lrem: {
            auto divisor = frame.pop<s8>();
            auto dividend = frame.pop<s8>();
            if (divisor == 0) {
                return throw_new_ArithmeticException_division_by_zero(thread, frame);
            }
            auto result = dividend - mul_overflow(div_overflow(dividend, divisor), divisor);
            frame.push<s8>(result);
            break;
        }
        case OpCodes::frem: {
            auto divisor = frame.pop<float>();
            auto dividend = frame.pop<float>();
            auto result = std::fmod(dividend, divisor);
            frame.push<float>(result);
            break;
        }
        case OpCodes::drem: {
            auto divisor = frame.pop<double>();
            auto dividend = frame.pop<double>();
            auto result = std::fmod(dividend, divisor);
            frame.push<double>(result);
            break;
        }
        case OpCodes::ineg: {
            auto a = frame.pop<s4>();
            frame.push<s4>(sub_overflow(static_cast<s4>(0), a));
            break;
        }
        case OpCodes::lneg: {
            auto a = frame.pop<s8>();
            frame.push<s8>(sub_overflow(static_cast<s8>(0), a));
            break;
        }
        case OpCodes::fneg: {
            auto a = frame.pop<float>();
            frame.push<float>(-a);
            break;
        }
        case OpCodes::dneg: {
            auto a = frame.pop<double>();
            frame.push<double>(-a);
            break;
        }
        case OpCodes::ishl: {
            auto shift = frame.pop<s4>() & 0x1F;
            auto value = frame.pop<s4>();
            frame.push<s4>(value << shift);
            break;
        }
        case OpCodes::lshl: {
            auto shift = frame.pop<s4>() & 0x3F;
            auto value = frame.pop<s8>();
            frame.push<s8>(value << shift);
            break;
        }
        case OpCodes::ishr: {
            auto shift = frame.pop<s4>() & 0x1F;
            auto value = frame.pop<s4>();
            frame.push<s4>(value >> shift);
            break;
        }
        case OpCodes::lshr: {
            auto shift = frame.pop<s4>() & 0x3F;
            auto value = frame.pop<s8>();
            frame.push<s8>(value >> shift);
            break;
        }
        case OpCodes::iushr: {
            auto shift = frame.pop<s4>() & 0x1F;
            auto value = frame.pop<s4>();
            // C++20 always performs arithmetic shifts, so the top bits need to be cleared out afterwards
            frame.push<s4>((value >> shift) &
                           (future::bit_cast<s4>(std::numeric_limits<u4>::max() >> static_cast<u4>(shift)))
            );
            break;
        }
        case OpCodes::lushr: {
            auto shift = frame.pop<s4>() & 0x3F;
            auto value = frame.pop<s8>();
            // C++20 always performs arithmetic shifts, so the top bits need to be cleared out afterwards
            frame.push<s8>((value >> shift) &
                           (future::bit_cast<s8>(std::numeric_limits<u8>::max() >> static_cast<u8>(shift)))
            );
            break;
        }
        case OpCodes::iand:
            frame.push<s4>(frame.pop<s4>() & frame.pop<s4>());
            break;
        case OpCodes::land:
            frame.push<s8>(frame.pop<s8>() & frame.pop<s8>());
            break;
        case OpCodes::ior:
            frame.push<s4>(frame.pop<s4>() | frame.pop<s4>());
            break;
        case OpCodes::lor:
            frame.push<s8>(frame.pop<s8>() | frame.pop<s8>());
            break;
        case OpCodes::ixor:
            frame.push<s4>(frame.pop<s4>() ^ frame.pop<s4>());
            break;
        case OpCodes::lxor:
            frame.push<s8>(frame.pop<s8>() ^ frame.pop<s8>());
            break;
        case OpCodes::iinc: {
            auto local = frame.consume_u1();
            auto value = static_cast<s4>(static_cast<s2>(future::bit_cast<s1>(frame.consume_u1())));
            auto result = add_overflow(frame.locals[local].s4, value);
            frame.locals[local] = Value(result);
            break;
        }

            /* ======================= Conversions ======================= */
        case OpCodes::i2l:
            frame.push<s8>(static_cast<s8>(frame.pop<s4>()));
            break;
        case OpCodes::i2f:
            frame.push<float>(static_cast<float>(frame.pop<s4>()));
            break;
        case OpCodes::i2d:
            frame.push<double>(static_cast<double>(frame.pop<s4>()));
            break;
        case OpCodes::l2i:
            frame.push<s4>(static_cast<s4>(frame.pop<s8>()));
            break;
        case OpCodes::l2f:
            frame.push<float>(static_cast<float>(frame.pop<s8>()));
            break;
        case OpCodes::l2d:
            frame.push<double>(static_cast<double>(frame.pop<s8>()));
            break;
        case OpCodes::f2i: {
            frame.push<s4>(floating_to_integer<float, s4>(frame.pop<float>()));
            break;
        }
        case OpCodes::f2l:
            frame.push<s8>(floating_to_integer<float, s8>(frame.pop<float>()));
            break;
        case OpCodes::f2d:
            frame.push<double>(static_cast<double>(frame.pop<float>()));
            break;
        case OpCodes::d2i:
            frame.push<s4>(floating_to_integer<double, s4>(frame.pop<double>()));
            break;
        case OpCodes::d2l:
            frame.push<s8>(floating_to_integer<double, s8>(frame.pop<double>()));
            break;
        case OpCodes::d2f:
            frame.push<float>(static_cast<float>(frame.pop<double>()));
            break;
        case OpCodes::i2b:
            frame.push<s4>(static_cast<s4>(static_cast<s1>(frame.pop<s4>())));
            break;
        case OpCodes::i2c:
            frame.push<s4>(static_cast<s4>(static_cast<u2>(frame.pop<s4>())));
            break;
        case OpCodes::i2s:
            frame.push<s4>(static_cast<s4>(static_cast<s2>(frame.pop<s4>())));
            break;

            /* ======================= Comparisons ======================= */
        case OpCodes::lcmp: {
            auto b = frame.pop<s8>();
            auto a = frame.pop<s8>();
            if (a > b) {
                frame.push<s4>(1);
            } else if (a == b) {
                frame.push<s4>(0);
            } else {
                frame.push<s4>(-1);
            }
            break;
        }
        case OpCodes::fcmpl:
        case OpCodes::fcmpg: {
            // TODO "value set conversion" ?
            auto b = frame.pop<float>();
            auto a = frame.pop<float>();
            if (a > b) {
                frame.push<s4>(1);
            } else if (a == b) {
                frame.push<s4>(0);
            } else if (a < b) {
                frame.push<s4>(-1);
            } else {
                // at least one of a' or b' is NaN
                frame.push<s4>(static_cast<OpCodes>(opcode) == OpCodes::fcmpg ? -1 : 1);
            }
            break;
        }
        case OpCodes::dcmpl:
        case OpCodes::dcmpg: {
            // TODO "value set conversion" ?
            auto b = frame.pop<double>();
            auto a = frame.pop<double>();
            if (a > b) {
                frame.push<s4>(1);
            } else if (a == b) {
                frame.push<s4>(0);
            } else if (a < b) {
                frame.push<s4>(-1);
            } else {
                // at least one of a' or b' is NaN
                frame.push<s4>(static_cast<OpCodes>(opcode) == OpCodes::dcmpl ? -1 : 1);
            }
            break;
        }
        case OpCodes::ifeq:
            execute_comparison(frame, frame.pop<s4>() == 0);
            return;
        case OpCodes::ifne:
            execute_comparison(frame, frame.pop<s4>() != 0);
            return;
        case OpCodes::iflt:
            execute_comparison(frame, frame.pop<s4>() < 0);
            return;
        case OpCodes::ifge:
            execute_comparison(frame, frame.pop<s4>() >= 0);
            return;
        case OpCodes::ifgt:
            execute_comparison(frame, frame.pop<s4>() > 0);
            return;
        case OpCodes::ifle:
            execute_comparison(frame, frame.pop<s4>() <= 0);
            return;
        case OpCodes::if_icmpeq:
            execute_comparison(frame, frame.pop<s4>() == frame.pop<s4>());
            return;
        case OpCodes::if_icmpne:
            execute_comparison(frame, frame.pop<s4>() != frame.pop<s4>());
            return;
        case OpCodes::if_icmplt:
            execute_comparison(frame, frame.pop<s4>() > frame.pop<s4>());
            return;
        case OpCodes::if_icmpge:
            execute_comparison(frame, frame.pop<s4>() <= frame.pop<s4>());
            return;
        case OpCodes::if_icmpgt:
            execute_comparison(frame, frame.pop<s4>() < frame.pop<s4>());
            return;
        case OpCodes::if_icmple:
            execute_comparison(frame, frame.pop<s4>() >= frame.pop<s4>());
            return;
        case OpCodes::if_acmpeq:
            execute_comparison(frame, frame.pop<Reference>() == frame.pop<Reference>());
            return;
        case OpCodes::if_acmpne:
            execute_comparison(frame, frame.pop<Reference>() != frame.pop<Reference>());
            return;

            /* ======================= Control =======================*/
        case OpCodes::goto_:
            goto_(frame);
            return;
        case OpCodes::jsr:
        case OpCodes::ret:
            throw std::runtime_error("jsr and ret are unsupported");
        case OpCodes::tableswitch: {
            auto pc = frame.pc;
            size_t opcode_address = pc;

            // skip opcode + 0-3 bytes of padding
            pc = (pc + 4) & -4ul;

            s4 default_ = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 low = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 high = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            assert(low <= high);

//            s4 count  = high - low + 1;
            s4 index = frame.pop<s4>();

            s4 offset;
            if (index < low || index > high) {
                offset = default_;
            } else {
                pc += 4 * static_cast<u4>(index - low);
                offset = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            }

            frame.pc = opcode_address + static_cast<size_t>(static_cast<ssize_t>(offset));
            return;
        }
        case OpCodes::lookupswitch: {
            auto pc = frame.pc;
            size_t opcode_address = pc;

            // skip opcode + 0-3 bytes of padding
            pc = (pc + 4) & -4ul;

            s4 default_ = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            s4 npairs = static_cast<s4>((code[pc] << 24) | (code[pc + 1] << 16) | (code[pc + 2] << 8) | code[pc + 3]);
            pc += 4;
            assert(npairs >= 0);

            s4 key = frame.pop<s4>();
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

            frame.pc = opcode_address + static_cast<size_t>(static_cast<ssize_t>(offset));
            return;
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
            pop_frame(thread, frame, should_exit);
            return;
        }


            /* ======================= References ======================= */
        case OpCodes::getstatic:
        case OpCodes::putstatic:
        case OpCodes::getfield:
        case OpCodes::putfield: {
            u2 index = frame.read_u2();
            auto field = frame.constant_pool->get<CONSTANT_Fieldref_info>(index);

            if (!field.resolved) {
                if (resolve_class(field.class_)) {
                    return;
                }

                if (resolve_field(field.class_->clazz, &field, thread.current_exception)) {
                    return;
                }
                if (thread.current_exception != JAVA_NULL)
                    throw std::runtime_error(
                            "field not found: " + field.class_->name->value + "." + field.name_and_type->name->value +
                            " " + field.name_and_type->descriptor->value);
                assert(field.resolved);
            }
            frame.pc += 2;

            switch (static_cast<OpCodes>(opcode)) {
                case OpCodes::getstatic: {
                    if (!field.is_static)
                        throw std::runtime_error("field is not static");
                    if (initialize_class(field.value_clazz, thread, frame)) {
                        return;
                    }
                    auto value = field.value_clazz->static_field_values[field.index];
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
                    if (initialize_class(field.value_clazz, thread, frame)) {
                        return;
                    }
                    Value value;
                    if (field.category == ValueCategory::C1) {
                        value = frame.pop();
                        if (field.is_boolean)
                            value.s4 = value.s4 & 1;
                    } else {
                        value = frame.pop2();
                    }
                    field.value_clazz->static_field_values[field.index] = value;
                    break;
                }
                case OpCodes::getfield: {
                    if (field.is_static)
                        throw std::runtime_error("field is static");
                    auto objectref = frame.pop<Reference>();
                    if (objectref == JAVA_NULL) {
                        return throw_new(thread, frame, Names::java_lang_NullPointerException);
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
                    auto objectref = frame.pop<Reference>();
                    if (objectref == JAVA_NULL) {
                        return throw_new(thread, frame, Names::java_lang_NullPointerException);
                    }
                    objectref.data<Value>()[field.index] = value;
                    break;
                }
                default:
                    assert(false);
            }

            break;
        }
        case OpCodes::invokevirtual: {
            u2 method_index = frame.read_u2();
            auto &declared_method_ref = frame.constant_pool->get<CONSTANT_Methodref_info>(method_index).method;

            method_info *declared_method = declared_method_ref.method;

            if (declared_method == nullptr) {
                if (resolve_class(declared_method_ref.class_)) {
                    return;
                }

                if (method_resolution(declared_method_ref)) {
                    return;
                }
                declared_method = declared_method_ref.method;
            }

            auto object = frame.peek_at(declared_method->stack_slots_for_parameters - 1).reference;
            if (object == JAVA_NULL) {
                return throw_new(thread, frame, Names::java_lang_NullPointerException);
            }

            method_info *method;
            if (method_selection(object.object()->clazz, declared_method, method)) {
                return;
            }

            frame.invoke_length = 3;
            thread.stack.push_frame(frame, method);

            if (method->is_native()) {
                native_call(method, thread, frame, should_exit);
            }
            return;
        }
        case OpCodes::invokespecial: {
            u2 method_index = frame.read_u2();
            auto &ref = frame.constant_pool->table[method_index].variant;
            ClassInterface_Methodref *method_ref;
            if (auto m = std::get_if<CONSTANT_Methodref_info>(&ref)) {
                method_ref = &m->method;
            } else {
                method_ref = &std::get_if<CONSTANT_InterfaceMethodref_info>(&ref)->method;
            }

            method_info *method = method_ref->method;
            if (method == nullptr) {
                if (resolve_class(method_ref->class_)) {
                    return;
                }

                if (method_resolution(*method_ref)) {
                    return;
                }
                method = method_ref->method;
            }

            frame.invoke_length = 3;
            thread.stack.push_frame(frame, method);

            if (method->is_native()) {
                native_call(method, thread, frame, should_exit);
            }
            return;
        }
        case OpCodes::invokestatic: {
            u2 method_index = frame.read_u2();
            auto &ref = frame.constant_pool->table[method_index].variant;
            ClassInterface_Methodref *method_ref;
            if (auto m = std::get_if<CONSTANT_Methodref_info>(&ref)) {
                method_ref = &m->method;
            } else {
                method_ref = &std::get_if<CONSTANT_InterfaceMethodref_info>(&ref)->method;
            }

            // TODO this is hardcoded for now
            if (method_ref->class_->name->value == "java/lang/System" &&
                method_ref->name_and_type->name->value == "exit" &&
                method_ref->name_and_type->descriptor->value == "(I)V") {
                exit(EXIT_FAILURE);
                return;
            } else if (method_ref->class_->name->value == "java/lang/System" &&
                       method_ref->name_and_type->name->value == "loadLibrary" &&
                       method_ref->name_and_type->descriptor->value == "(Ljava/lang/String;)V") {
                // Ignore for now
                frame.pc += 2;
                break;
            }

            method_info *method = method_ref->method;
            ClassFile *clazz = method_ref->class_->clazz;

            if (method == nullptr) {
                if (resolve_class(method_ref->class_)) {
                    return;
                }

                clazz = method_ref->class_->clazz;
                if (initialize_class(clazz, thread, frame)) {
                    return;
                }

                if (method_resolution(*method_ref)) {
                    return;
                }
                method = method_ref->method;
            }

            frame.invoke_length = 3;
            thread.stack.push_frame(frame, method);

            if (method->is_native()) {
                native_call(method, thread, frame, should_exit);
            }
            return;
        }
        case OpCodes::invokeinterface: {
            u2 method_index = frame.read_u2();
            auto &declared_method_ref = frame.constant_pool->get<CONSTANT_InterfaceMethodref_info>(
                    method_index).method;

            method_info *declared_method = declared_method_ref.method;

            if (declared_method == nullptr) {
                if (resolve_class(declared_method_ref.class_)) {
                    return;
                }

                // TODO we will need some special handling for methods on Object here
                if (method_resolution(declared_method_ref)) {
                    return;
                }
                declared_method = declared_method_ref.method;
            }

            auto object = frame.peek_at(declared_method->stack_slots_for_parameters - 1).reference;

            method_info *method;
            if (method_selection(object.object()->clazz, declared_method, method)) {
                return;
            }

            // The two bytes after the method index in the bytecode are irrelevant and unused
            frame.invoke_length = 5;
            thread.stack.push_frame(frame, method);

            if (method->is_native()) {
                native_call(method, thread, frame, should_exit);
            }
            return;
        }
        case OpCodes::new_: {
            u2 index = frame.read_u2();
            auto &class_info = frame.constant_pool->get<CONSTANT_Class_info>(index);

            if (resolve_class(&class_info)) {
                return;
            }

            frame.pc += 2;
            auto clazz = class_info.clazz;
            if (initialize_class(clazz, thread, frame)) {
                return;
            }

            auto reference = Heap::get().new_instance(clazz);
            frame.push<Reference>(reference);

            // TODO: the next two instructions are probably dup+invokespecial. We could optimize for that pattern.
            break;
        }
        case OpCodes::newarray: {
            s4 count = frame.pop<s4>();
            if (count < 0) {
                throw std::runtime_error("TODO NegativeArraySizeException");
            }
            s4 type = frame.consume_u1();

            switch (static_cast<ArrayPrimitiveTypes>(type)) {
                case ArrayPrimitiveTypes::T_INT: {
                    auto reference = Heap::get().new_array<s4>(BootstrapClassLoader::primitive(Primitive::Int).array,
                                                               count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_BOOLEAN: {
                    auto reference = Heap::get().new_array<s1>(
                            BootstrapClassLoader::primitive(Primitive::Boolean).array, count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_CHAR: {
                    auto reference = Heap::get().new_array<u2>(BootstrapClassLoader::primitive(Primitive::Char).array,
                                                               count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_FLOAT: {
                    auto reference = Heap::get().new_array<float>(
                            BootstrapClassLoader::primitive(Primitive::Float).array, count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_DOUBLE: {
                    auto reference = Heap::get().new_array<double>(
                            BootstrapClassLoader::primitive(Primitive::Double).array, count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_BYTE: {
                    auto reference = Heap::get().new_array<s1>(BootstrapClassLoader::primitive(Primitive::Byte).array,
                                                               count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_SHORT: {
                    auto reference = Heap::get().new_array<s2>(BootstrapClassLoader::primitive(Primitive::Short).array,
                                                               count);
                    frame.push<Reference>(reference);
                    break;
                }
                case ArrayPrimitiveTypes::T_LONG: {
                    auto reference = Heap::get().new_array<s8>(BootstrapClassLoader::primitive(Primitive::Long).array,
                                                               count);
                    frame.push<Reference>(reference);
                    break;
                }
            }
            break;
        }
        case OpCodes::anewarray: {
            u2 index = frame.read_u2();
            auto &class_info = frame.constant_pool->get<CONSTANT_Class_info>(index);
            if (resolve_class(&class_info)) {
                return;
            }
            frame.pc += 2;

            s4 count = frame.pop<s4>();
            if (count < 0) {
                throw std::runtime_error("TODO NegativeArraySizeException");
            }

            ClassFile *element = class_info.clazz;
            ClassFile *array_class = BootstrapClassLoader::get().load(element->as_array_element());

            auto reference = Heap::get().new_array<Reference>(array_class, count);
            frame.push<Reference>(reference);
            break;
        }
        case OpCodes::arraylength: {
            auto arrayref = frame.pop<Reference>();
            if (arrayref == JAVA_NULL) {
                return throw_new(thread, frame, Names::java_lang_NullPointerException);
            }
            frame.push<s4>(arrayref.object()->length);
            break;
        }

        case OpCodes::athrow: {
            auto value = frame.pop<Reference>();
            if (value == JAVA_NULL) {
                return throw_new(thread, frame, Names::java_lang_NullPointerException);
            }
            return throw_it(thread, value);
        }

        case OpCodes::checkcast: {
            u2 index = frame.read_u2();
            auto &class_info = frame.constant_pool->get<CONSTANT_Class_info>(index);

            auto objectref = frame.pop<Reference>();
            frame.push<Reference>(objectref);

            if (objectref != JAVA_NULL) {
                if (resolve_class(&class_info)) {
                    return;
                }

                if (!objectref.object()->clazz->is_instance_of(class_info.clazz)) {
                    return throw_new(thread, frame, Names::java_lang_ClassCastException);
                }
            }
            frame.pc += 2;
            break;
        }

        case OpCodes::instanceof: {
            u2 index = frame.read_u2();
            auto &class_info = frame.constant_pool->get<CONSTANT_Class_info>(index);

            auto objectref = frame.pop<Reference>();
            if (objectref == JAVA_NULL) {
                frame.push<s4>(0);
            } else {
                frame.push<Reference>(objectref);
                if (resolve_class(&class_info)) {
                    return;
                }
                frame.pop<Reference>();

                frame.push<bool>(objectref.object()->clazz->is_instance_of(class_info.clazz));
            }
            frame.pc += 2;
            break;
        }
        case OpCodes::monitorenter:
        case OpCodes::monitorexit: {
            // TODO noop for now
            auto reference = frame.pop<Reference>();
            if (reference == JAVA_NULL) {
                return throw_new(thread, frame, Names::java_lang_NullPointerException);
            }
            break;
        }

        case OpCodes::wide: {
            u1 type = frame.consume_u1();
            u2 index = frame.consume_u2();
            auto &local = frame.locals[index];

            switch (static_cast<OpCodes>(type)) {
                case OpCodes::iload:
                    frame.push<s4>(local.s4);
                    break;
                case OpCodes::lload:
                    frame.push<s8>(local.s8);
                    break;
                case OpCodes::fload:
                    frame.push<float>(local.float_);
                    break;
                case OpCodes::dload:
                    frame.push<double>(local.double_);
                    break;
                case OpCodes::aload:
                    frame.push<Reference>(local.reference);
                    break;

                case OpCodes::istore:
                    local = Value(frame.pop<s4>());
                    break;
                case OpCodes::lstore:
                    local = Value(frame.pop<s8>());
                    break;
                case OpCodes::fstore:
                    local = Value(frame.pop<float>());
                    break;
                case OpCodes::dstore:
                    local = Value(frame.pop<double>());
                    break;
                case OpCodes::astore:
                    local = Value(frame.pop<Reference>());
                    break;

                case OpCodes::iinc: {
                    s4 constant = future::bit_cast<s2>(frame.consume_u2());
                    local.s4 += constant;
                    frame.pc++;
                    return;
                }

                case OpCodes::ret:
                    throw std::runtime_error("jsr and ret are unsupported");
                default:
                    abort();
            }
            break;
        }
        case OpCodes::multianewarray: {
            u2 index = frame.read_u2();
            auto &class_info = frame.constant_pool->get<CONSTANT_Class_info>(index);
            if (resolve_class(&class_info)) {
                return;
            }
            frame.pc += 2;

            u1 dimensions = frame.consume_u1();
            assert(dimensions >= 1);
            // The last entry is the "root" dimension. So reversed compared to int[a][b][c]
            // TODO cache the vector in the current thread or create the span directly from frame.operands (with a stride of 2) to avoid memory allocations
            std::vector<s4> counts;
            counts.reserve(dimensions);
            for (size_t i = 0; i < dimensions; i++) {
                auto count = frame.pop<s4>();
                counts.push_back(count);
                if (count < 0) {
                    throw std::runtime_error("TODO NegativeArraySizeException");
                }
            }

            auto reference = Heap::get().new_array<Reference>(class_info.clazz, counts.back());
            fill_multi_array(reference, class_info.clazz->array_element_type,
                             std::span(counts).subspan(0, counts.size() - 1));
            frame.push<Reference>(reference);
            break;
        }
        case OpCodes::ifnull:
            execute_comparison(frame, frame.pop<Reference>() == JAVA_NULL);
            return;
        case OpCodes::ifnonnull:
            execute_comparison(frame, frame.pop<Reference>() != JAVA_NULL);
            return;
        case OpCodes::goto_w: {
            auto pc = frame.pc;
            s4 offset = static_cast<s4>((code[pc + 1] << 24) | (code[pc + 2] << 16) | (code[pc + 3] << 8) |
                                        code[pc + 4]);
            frame.pc += static_cast<size_t>(static_cast<ssize_t>(offset));
            return;
        }
        case OpCodes::jsr_w:
            throw std::runtime_error("jsr and ret are unsupported");

        default:
            throw std::runtime_error(
                    "Unimplemented/unknown opcode " + std::to_string(opcode) + " at " + std::to_string(frame.pc)
            );
    }

    frame.pc++;
}

static void execute_comparison(Frame &frame, bool condition) {
    if (condition) {
        return goto_(frame);
    } else {
        frame.pc += 3;
    }
}

static void goto_(Frame &frame) {
    u2 offset_u = frame.read_u2();
    auto offset = future::bit_cast<s2>(offset_u);
    frame.pc = static_cast<size_t>(static_cast<long>(frame.pc) + offset);
}


static void handle_throw(Thread &thread, Frame &frame, Reference exception, bool &should_exit) {
    assert(exception != JAVA_NULL);
    auto obj = exception.object();

    for (;;) {
        auto &exception_table = frame.method->code_attribute->exception_table;

        auto handler_iter = std::find_if(exception_table.begin(),
                                         exception_table.end(),
                                         [&frame, obj](const ExceptionTableEntry &e) {
                                             if (e.start_pc <= frame.pc && frame.pc < e.end_pc) {
                                                 if (e.catch_type == 0) {
                                                     // "any"
                                                     return true;
                                                 } else {
                                                     // TODO Don't compare by name but resolve the class instead,
                                                     // but without running the class initializer.
                                                     auto &clazz_name = frame.constant_pool->get<CONSTANT_Class_info>(
                                                             e.catch_type).name->value;
                                                     for (ClassFile *c = obj->clazz;; c = c->super_class) {
                                                         if (c->name() == clazz_name) { return true; }
                                                         if (c->super_class == nullptr) { break; }
                                                     }
                                                     return false;
                                                 }
                                             }
                                             return false;
                                         }
        );

        if (handler_iter == std::end(exception_table)) {
            pop_frame(thread, frame, should_exit);
            if (should_exit) {
                // the native caller has to deal with thread.current_exception
                return;
            }
        } else {
            // Push exception back on stack, continue normal execution.
            frame.clear();
            frame.push<Reference>(exception);
            frame.pc = handler_iter->handler_pc;
            thread.current_exception = JAVA_NULL;
            return;
        }
    }
}

static void pop_frame(Thread &thread, Frame &frame, bool &should_exit) {
    thread.stack.memory_used = frame.previous_stack_memory_usage;

    if (frame.is_root_frame) {
        should_exit = true;
    } else {
        frame = thread.stack.pop_frame();

        if (thread.current_exception == JAVA_NULL) {
            frame.pc += frame.invoke_length;
        }
    }
}


Frame::Frame(Stack &stack, method_info *method, size_t operand_stack_top, bool is_root_frame)
        : method(method),
          code(nullptr),
          constant_pool(method != nullptr ? &method->clazz->constant_pool : nullptr),
          operands_top(0),
          previous_stack_memory_usage(stack.memory_used),
          pc(0),
          invoke_length(0),
          is_root_frame(is_root_frame) {
    //assert(method->code_attribute->max_locals >= method->parameter_count);
    assert(stack.memory_used >= method->stack_slots_for_parameters);
    assert(operand_stack_top >= method->stack_slots_for_parameters);

    size_t first_local_index = operand_stack_top - method->stack_slots_for_parameters;

    if (method->code_attribute != nullptr) {
        code = &method->code_attribute->code;

        first_operand_index = first_local_index + method->code_attribute->max_locals;
        stack.memory_used = first_operand_index + method->code_attribute->max_stack;

        locals = {&stack.memory[first_local_index], method->code_attribute->max_locals};
        operands = {&stack.memory[first_operand_index], method->code_attribute->max_stack};
    } else {
        assert(method->is_native());

        first_operand_index = operand_stack_top;
        stack.memory_used = operand_stack_top;

        locals = {&stack.memory[first_local_index], method->stack_slots_for_parameters};
        operands = {&stack.memory[first_operand_index], (size_t) 0};
    }

    // TODO think about value set conversion
    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-2.html#jvms-2.8.3
}

template<typename Element>
void array_store(Thread &thread, Frame &frame) {
    auto value = frame.pop<Element>();
    auto index = frame.pop<s4>();

    auto arrayref = frame.pop<Reference>();
    if (arrayref == JAVA_NULL) {
        return throw_new(thread, frame, Names::java_lang_NullPointerException);
    }

    if (index < 0 || index >= arrayref.object()->length) {
        throw std::runtime_error("TODO ArrayIndexOutOfBoundsException");
    }

    arrayref.data<Element>()[index] = value;
}

template<typename Element>
void array_load(Thread &thread, Frame &frame) {
    auto index = frame.pop<s4>();

    auto arrayref = frame.pop<Reference>();
    if (arrayref == JAVA_NULL) {
        return throw_new(thread, frame, Names::java_lang_NullPointerException);
    }

    if (index < 0 || index >= arrayref.object()->length) {
        throw std::runtime_error("TODO ArrayIndexOutOfBoundsException");
    }

    frame.push<Element>(arrayref.data<Element>()[index]);
}

void fill_multi_array(Reference &reference, ClassFile *element_type, const std::span<s4> &counts) {
    s4 count = counts.back();
    // If any count value is zero, no subsequent dimensions are allocated
    if (count == 0) return;

    for (s4 i = reference.object()->length - 1; i >= 0; i--) {
        auto child = Heap::get().new_array<Reference>(element_type, count);
        if (counts.size() > 1) {
            fill_multi_array(child, element_type->array_element_type, counts.subspan(0, counts.size() - 1));
        }
        reference.data<Reference>()[i] = child;
    }
}

static void
resolve_method_interfaces(ClassFile *clazz, const std::string &name, const std::string &descriptor,
                          method_info *&out_method_max_specific, method_info *&out_method_fallback) {
    if (clazz->is_interface()) {
        for (auto &m : clazz->methods) {
            if (m.name_index->value == name &&
                m.descriptor_index->value == descriptor &&
                !m.is_private() && !m.is_static()) {
                // arbitrarily use the last matching method as a fallback
                out_method_fallback = &m;

                if (!m.is_abstract()) {
                    if (out_method_max_specific == nullptr) {
                        out_method_max_specific = &m;
                    } else {
                        if (clazz->is_subclass_of(out_method_max_specific->clazz)) {
                            // more specific class found
                            out_method_max_specific = &m;
                        }
                    }
                }
            }
        }
    }

    for (auto &interface : clazz->interfaces) {
        resolve_method_interfaces(interface->clazz, name, descriptor, out_method_max_specific, out_method_fallback);
    }
    if (clazz->super_class) {
        resolve_method_interfaces(clazz->super_class, name, descriptor, out_method_max_specific, out_method_fallback);
    }
}

method_info *method_resolution(ClassFile *clazz, std::string const &name, std::string const &descriptor) {
    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.3.3

    // 2.
    for (ClassFile *c = clazz; c != nullptr; c = c->super_class) {
        for (auto &m : c->methods) {
            if (m.name_index->value == name &&
                m.descriptor_index->value == descriptor) {
                return &m;
            }
        }
    }

    // 3.
    method_info *out_method_max_specific = nullptr;
    method_info *out_method_fallback = nullptr;
    resolve_method_interfaces(clazz, name, descriptor, out_method_max_specific, out_method_fallback);

    if (out_method_max_specific != nullptr) {
        return out_method_max_specific;
    } else if (out_method_fallback != nullptr) {
        return out_method_fallback;
    }

    // TODO throw the appropriate JVM exception instead
    throw std::runtime_error(
            "Couldn't find method (static): " + name + descriptor + " in class " + clazz->name());

}

[[nodiscard]] static bool method_resolution(ClassInterface_Methodref &method) {
    auto result = method_resolution(method.class_->clazz, method.name_and_type->name->value,
                                    method.name_and_type->descriptor->value);

    if (result == nullptr) {
        return true;
    } else {
        method.method = result;
        return false;
    }
}

[[nodiscard]] bool
method_selection(ClassFile *dynamic_class, method_info *declared_method, method_info *&out_method) {
    assert(!declared_method->is_static());
    // https://docs.oracle.com/javase/specs/jvms/se16/html/jvms-5.html#jvms-5.4.6
    // 1.
    if (declared_method->is_private()) {
        out_method = declared_method;
        return false;
    }

    const auto &name = declared_method->name_index->value;
    const auto &descriptor = declared_method->descriptor_index->value;
    // 2. (1 + 2)
    for (ClassFile *clazz = dynamic_class; clazz != nullptr; clazz = clazz->super_class) {
        for (auto &m : clazz->methods) {
            // "can override" according to ??5.4.5
            if (m.name_index->value == name &&
                m.descriptor_index->value == descriptor &&
                !m.is_private() && !m.is_static() &&
                (
                        m.is_protected() || m.is_public()
                        ||
                        declared_method->clazz->package_name == clazz->package_name
                        // TODO || (bullet point 3b)
                )
                    ) {
                out_method = &m;
                return false;
            }
        }
    }

    // 2. (3)
    method_info *out_method_max_specific = nullptr;
    method_info *out_method_fallback = nullptr;
    resolve_method_interfaces(dynamic_class, name, descriptor, out_method_max_specific, out_method_fallback);

    if (out_method_max_specific != nullptr && !out_method_max_specific->is_abstract()) {
        out_method = out_method_max_specific;
        return false;
    } else if (out_method_fallback != nullptr) {
        out_method = out_method_fallback;
        return false;
    }

    // TODO throw the appropriate JVM exception instead
    // TODO shouldn't this be impossible as long as declared isn't abstract?
    throw std::runtime_error("Couldn't find method (virtual): " + name + descriptor);
}

void native_call(method_info *method, Thread &thread, Frame &frame, bool &should_exit) {
    // during native calls we push the current frame
    thread.stack.push_frame(frame);

    if (!method->native_function) {
        auto *function_pointer = get_native_function_pointer(method);
        if (function_pointer == nullptr) {
            // TODO throw an exception and return
            abort();
        }
        method->native_function = NativeFunction(method, function_pointer);
    }
    NativeFunction &native = *method->native_function;

    void *jni_env_argument = thread.jni_env;
    ClassFile *class_argument = method->clazz;
    bool use_class_argument = method->is_static();

    Value return_value;
    if (native.argument_count() <= 13) {
        void *arguments[13];
        native.prepare_argument_pointers(arguments,
                                         &jni_env_argument, &class_argument, use_class_argument, frame.locals);
        return_value = native.call(arguments);
    } else {
        std::vector<void *> arguments;
        arguments.resize(native.argument_count());
        native.prepare_argument_pointers(arguments.data(),
                                         &jni_env_argument, &class_argument, use_class_argument, frame.locals);
        return_value = native.call(arguments.data());
    }

    if (thread.current_exception != JAVA_NULL) {
        // TODO handle excaption
    }

    if (method->return_category != 0) {
        frame.locals[0] = return_value;
    }

    // restore the native frame
    frame = thread.stack.pop_frame();

    // remove the native frame
    pop_frame(thread, frame, should_exit);
}

