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
    thread.stack.frames.push_back(frame);
    throw_it(thread, it);
}

void throw_new(Thread &thread, Frame &frame, const char *name);

void throw_new(Thread &thread, const char *name);

void fill_in_stack_trace(Stack &stack, Reference throwable);

void init_stack_trace_element_array(Reference elements, Reference throwable);

#endif //SCHOKOVM_EXCEPTIONS_HPP
