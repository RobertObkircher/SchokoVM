#include <iostream>

#include "classfile.hpp"
#include "classloading.hpp"
#include "exceptions.hpp"

void throw_new(Thread &thread, Frame &frame, const char *name) {
    ClassFile *clazz = BootstrapClassLoader::get().load(name);
    if (clazz == nullptr) {
        std::cerr << "Attempted to throw unknown class " << name << "\n";
        abort();
    }

    if (resolve_class(clazz->this_class)) {
        return;
    }

    assert(clazz->is_subclass_of(BootstrapClassLoader::constants().java_lang_Throwable));

    if (initialize_class(clazz, thread, frame)) {
        return;
    }

    Reference ref = Heap::get().new_instance(clazz);

    // TODO what if heap allocation fails?
    // TODO call constructor
    // TODO jni Throw, ThrowNew, NewObject

    thread.current_exception = ref;
}
