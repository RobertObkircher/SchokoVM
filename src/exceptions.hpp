#ifndef SCHOKOVM_EXCEPTIONS_HPP
#define SCHOKOVM_EXCEPTIONS_HPP

struct Thread;
struct Frame;

void throw_new(Thread &thread, Frame &frame, const char *name);

#endif //SCHOKOVM_EXCEPTIONS_HPP
