#ifndef SCHOKOVM_EXCEPTIONS_HPP
#define SCHOKOVM_EXCEPTIONS_HPP

#include "classloading.hpp"
struct Frame;
struct Stack;
struct Thread;

inline void throw_it(Thread &thread, Reference it) {
    assert(it != JAVA_NULL);
    assert(it.object()->clazz->is_subclass_of(BootstrapClassLoader::constants().java_lang_Throwable));
    assert(thread.current_exception == JAVA_NULL);
    thread.current_exception = it;
}

inline void throw_it(Thread &thread, Frame &frame, Reference it) {
    thread.stack.push_frame(frame);
    throw_it(thread, it);
}

void throw_new(Thread &thread, Frame &frame, const char *name, const char *message = nullptr);

void throw_new(Thread &thread, ClassFile *clazz, const char *message = nullptr);

void fill_in_stack_trace(Stack &stack, Reference throwable);

void init_stack_trace_element_array(Reference elements, Reference throwable);

void throw_new_ArithmeticException_division_by_zero(Thread &thread, Frame &frame);

#endif //SCHOKOVM_EXCEPTIONS_HPP
