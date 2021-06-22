#include <cassert>
#include <iostream>
#include <variant>

#include "classfile.hpp"
#include "classloading.hpp"
#include "exceptions.hpp"

void throw_new(Thread &thread, Frame &frame, const char *name, const char *message) {
    thread.stack.push_frame(frame);
    throw_new(thread, name, message);
}

void throw_new(Thread &thread, const char *name, const char *message) {
    ClassFile *clazz = BootstrapClassLoader::get().load(name);
    if (clazz == nullptr) {
        std::cerr << "Attempted to throw unknown class " << name << "\n";
        abort();
    }

    if (resolve_class(clazz->this_class)) {
        return;
    }

    assert(clazz->is_subclass_of(BootstrapClassLoader::constants().java_lang_Throwable));

    if (initialize_class(clazz, thread)) {
        return;
    }

    Reference ref = Heap::get().new_instance(clazz);
    jmethodID init = thread.jni_env->GetMethodID((jclass) clazz, "<init>",
                                                 message == nullptr ? "()V" : "(Ljava/lang/String;)V");
    if (init == nullptr) {
        throw std::runtime_error("Couldn't find <init>");
    }
    if (message == nullptr) {
        thread.jni_env->CallNonvirtualVoidMethod((jobject) ref.memory, (jclass) clazz, init);
    } else {
        auto string = Heap::get().make_string(message);
        thread.jni_env->CallNonvirtualVoidMethod((jobject) ref.memory, (jclass) clazz, init, (jstring) string.memory);
    }

    thread.current_exception = ref;
}

void fill_in_stack_trace(Stack &stack, Reference throwable) {
    assert(throwable != JAVA_NULL);
    assert(throwable.object()->clazz->is_subclass_of(BootstrapClassLoader::constants().java_lang_Throwable));

    size_t ignored = 2; // fillInStackTrace (native method) + Throwable.fillInStackTrace (non native method)
    for (auto i = static_cast<ssize_t>(stack.frames.size() - 1 - ignored); i >= 0; --i) {
        auto const &frame = stack.frames[static_cast<size_t>(i)];
        if (frame.method->name_index->value != "<init>") {
            throw std::runtime_error("Fill in stack trace is expected to be called in an initializer");
        }
        ++ignored; // ignore Throwable/Exception initializers

        if (frame.clazz == throwable.object()->clazz) {
            break;
        }
    }

    auto root = std::find_if(stack.frames.rbegin() + ignored, stack.frames.rend(),
                             [](Frame const &frame) { return frame.is_root_frame; });
    if (root == stack.frames.rend()) {
        throw std::runtime_error("Can't fill in stacktrace because there no root frame was found!");
    }

    auto count = static_cast<size_t>(std::distance(stack.frames.rbegin() + ignored, root)) + 1;

    // TODO what type should this be? for now we just copy the frames
    auto array_class = BootstrapClassLoader::constants().java_lang_Object;
    auto array = Heap::get().new_array<Frame>(array_class, static_cast<s4>(count));

    for (size_t i = 0; i < count; ++i) {
        auto const &frame = stack.frames[stack.frames.size() - ignored - count + i];
        assert((i == 0) == frame.is_root_frame);
        array.data<Frame>()[count - 1 - i] = frame;
    }

    Reference &backtrace = throwable.data<Value>()[0].reference;
    s4 &depth = throwable.data<Value>()[4].s4;

    depth = static_cast<s4>(count);
    backtrace = array;
}

static Reference init_stack_trace_element(Frame const &frame, Reference element) {
    Reference &declaringClassObject = element.data<Value>()[0].reference; // Class
    Reference &classLoaderName = element.data<Value>()[1].reference; // Class
    Reference &moduleName = element.data<Value>()[2].reference; // String
    Reference &moduleVersion = element.data<Value>()[3].reference; // String
    Reference &declaringClass = element.data<Value>()[4].reference; // String
    Reference &methodName = element.data<Value>()[5].reference; // String
    Reference &fileName = element.data<Value>()[6].reference; // String
    s4 &lineNumber = element.data<Value>()[7].s4; // int

    declaringClassObject = Reference{frame.clazz};
    classLoaderName = JAVA_NULL;
    moduleName = JAVA_NULL;
    moduleVersion = JAVA_NULL;

    declaringClass = Heap::get().make_string(frame.clazz->name());
    methodName = Heap::get().make_string(frame.method->name_index->value);

    auto source_file = std::find_if(frame.clazz->attributes.begin(), frame.clazz->attributes.end(),
                                    [](const attribute_info &a) {
                                        return std::holds_alternative<SourceFile_attribute>(a.variant);
                                    });
    if (source_file != std::end(frame.clazz->attributes)) {
        auto value = std::get<SourceFile_attribute>(source_file->variant).sourcefile_index->value;
        fileName = Heap::get().make_string(value);
    }

    lineNumber = -1;
    if (frame.method->is_native()) {
        lineNumber = -2;
    } else {
        for (const auto &attribute : frame.method->code_attribute->attributes) {
            auto table = std::get_if<LineNumberTable_attribute>(&attribute.variant);
            if (table != nullptr) {
                for (auto entry = table->line_number_table.rbegin();
                     entry != table->line_number_table.rend(); ++entry) {
                    if (entry->start_pc <= frame.pc) {
                        lineNumber = entry->line_number;
                        break;
                    }
                }
                if (lineNumber != -1) {
                    break;
                }
            }
        }
    }

    return element;
}

void init_stack_trace_element_array(Reference elements, Reference throwable) {
    assert(elements != JAVA_NULL);
    assert(throwable != JAVA_NULL);
    assert(throwable.object()->clazz->is_subclass_of(BootstrapClassLoader::constants().java_lang_Throwable));

    Value &backtrace = throwable.data<Value>()[0];
//    Value &stackTrace = throwable.data<Value>()[3];
    Value &depth = throwable.data<Value>()[4];

    auto count = static_cast<size_t>(depth.s4);
    for (size_t i = 0; i < count; ++i) {
        auto const &frame = backtrace.reference.data<Frame>()[i];
        auto element = elements.data<Reference>()[i];
        init_stack_trace_element(frame, element);
    }
}

void throw_new_ArithmeticException_division_by_zero(Thread &thread, Frame &frame) {
    throw_new(thread, frame, Names::java_lang_ArithmeticException, "/ by zero");
}
