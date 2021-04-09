#include "parser.hpp"

#include <utility>
#include <sstream>
#include "classfile.hpp"

ParseError::ParseError(std::string message) : message(std::move(message)) {}

const char *ParseError::what() const noexcept {
    return message.c_str();
}

Parser::Parser(std::istream &in) : in(in) {
    // TODO How to handle errors?
    // I think the follwoing enables exceptions for all of these.
    // Is this a good idea?
    in.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
}

ClassFile Parser::parse() {
    ClassFile result;

    result.magic = eat_u4();
    if (result.magic != 0xCAFEBABE)
        throw ParseError("expected 0xCAFEBABE, not " + std::to_string(result.magic));

    result.minor_version = eat_u2();
    result.major_version = eat_u2();

    if (result.major_version < 45 || result.major_version > 60) {
        std::stringstream ss;
        ss << "invalid major version: " << result.major_version;
        throw ParseError(ss.str());
    }
    if (result.major_version >= 56) {
        if (result.minor_version != 0 && result.minor_version != 65535) {
            std::stringstream ss;
            ss << "invalid minor version: " << result.minor_version;
            throw ParseError(ss.str());
        }
    }

    result.constant_pool_count = eat_u2();

//    struct ClassFile {
//        u2 constant_pool_count;
//        cp_info constant_pool[constant_pool_count - 1];
//        u2 access_flags;
//        u2 this_class;
//        u2 super_class;
//        u2 interfaces_count;
//        u2 interfaces[interfaces_count];
//        u2 fields_count;
//        field_info fields[fields_count];
//        u2 methods_count;
//        method_info methods[methods_count];
//        u2 attributes_count;
//        attribute_info attributes[attributes_count];
//    };

    return result;
}
