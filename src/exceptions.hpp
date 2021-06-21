#ifndef SCHOKOVM_EXCEPTIONS_HPP
#define SCHOKOVM_EXCEPTIONS_HPP

struct Frame;
struct Stack;
struct Thread;

void throw_new(Thread &thread, Frame &frame, const char *name);

void throw_new(Thread &thread, const char *name);

void fill_in_stack_trace(Stack &stack, Reference throwable);

void init_stack_trace_element_array(Reference elements, Reference throwable);

#endif //SCHOKOVM_EXCEPTIONS_HPP
