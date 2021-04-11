#include "parser.hpp"

#include <utility>
#include <sstream>
#include <vector>
#include <cassert>
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
    ClassFile result{};

    result.magic = eat_u4();
    if (result.magic != 0xCAFEBABE)
        throw ParseError("expected 0xCAFEBABE, not " + std::to_string(result.magic));

    result.minor_version = eat_u2();
    result.major_version = eat_u2();

    if (result.major_version < 45 || result.major_version > 60)
        throw ParseError(std::string("invalid major version: ") + std::to_string(result.major_version));
    if (result.major_version >= 56 && result.minor_version != 0 && result.minor_version != 65535)
        throw ParseError(std::string("invalid minor version: ") + std::to_string(result.minor_version));

    result.constant_pool = parse_constant_pool(result.major_version);

    result.access_flags = eat_u2();
    result.this_class = eat_cp_index();
    result.super_class = eat_cp_index();

    u2 interfaces_count = eat_u2();
    for (int i = 0; i < interfaces_count; ++i)
        result.interfaces.push_back(eat_cp_index());

    u2 fields_count = eat_u2();
    for (int i = 0; i < fields_count; ++i) {
        field_info field_info;
        field_info.access_flags = eat_u2();
        field_info.name_index = eat_cp_index();
        field_info.descriptor_index = eat_cp_index();
        field_info.attributes = parse_attributes();
        result.fields.push_back(field_info);
    }

    u2 methods_count = eat_u2();
    for (int i = 0; i < methods_count; ++i) {
        method_info method_info;
        method_info.access_flags = eat_u2();
        method_info.name_index = eat_cp_index();
        method_info.descriptor_index = eat_cp_index();
        method_info.attributes = parse_attributes();
        result.methods.push_back(method_info);
    }

    result.attributes = parse_attributes();

    if (!in.eof())
        throw ParseError("Expected EOF");

    return result;
}

// TODO check the expected tag, not just the range and possibly get rid of the constant_pool_count member variable
// Inside parse_constant_pool we would have to defer checking the tag.
u2 Parser::eat_cp_index() {
    u2 index = eat_u2();
    if (index == 0 || index >= constant_pool_count)
        throw ParseError("Invalid constant pool index");
    return index;
}

ConstantPool Parser::parse_constant_pool(u2 major_version) {
    ConstantPool result{};

    constant_pool_count = eat_u2();
    result.table.reserve(constant_pool_count);

    result.table.push_back(cp_info{CONSTANT_Invalid, {}});

    for (size_t cp_index = 1; cp_index < constant_pool_count; ++cp_index) {
        u1 tag = eat_u1();

        cp_info info{};
        info.tag = static_cast<CpTag>(tag);
        u2 min_classfile_format = 45; // 45.3
        switch (tag) {
            case CONSTANT_Utf8: {
                u2 length = eat_u2();
                std::string str;
                str.resize(length); // not reserve
                in.read(str.data(), length);
                for (size_t i = 0; i < length; ++i) {
                    u1 byte = (u1) str[i];
                    if (byte == 0 || byte >= 0xf0)
                        throw ParseError("Invalid byte in utf8 string: " + std::to_string((int) byte));
                }
                info.info.utf_8_info.index = result.utf8_strings.size();
                result.utf8_strings.push_back(str);
                break;
            }
            case CONSTANT_Integer:
                info.info.integer_info.bytes = eat_u4();
                break;
            case CONSTANT_Float: {
                u4 bytes = eat_u4();
                info.info.float_info.value = *reinterpret_cast<float *>(&bytes);
                break;
            }
            case CONSTANT_Long: {
                u4 high_bytes = eat_u4();
                u4 low_bytes = eat_u4();
                u8 bytes = (u8) high_bytes << 32 | low_bytes;
                info.info.long_info.value = bytes;
                result.table.push_back(info);
                ++cp_index;
                result.table.push_back(cp_info{CONSTANT_Invalid, {}});
                continue;
            }
            case CONSTANT_Double: {
                u4 high_bytes = eat_u4();
                u4 low_bytes = eat_u4();
                u8 bytes = (u8) high_bytes << 32 | low_bytes;
                info.info.double_info.value = *reinterpret_cast<double *>(&bytes);
                ++cp_index;
                result.table.push_back(cp_info{CONSTANT_Invalid, {}});
                continue;
            }
            case CONSTANT_Class:
                info.info.class_info.name_index = eat_cp_index();
                break;
            case CONSTANT_String:
                info.info.string_info.string_index = eat_cp_index();
                break;
            case CONSTANT_Fieldref:
                info.info.fieldref_info.class_index = eat_cp_index();
                info.info.fieldref_info.name_and_type_index = eat_cp_index();
                break;
            case CONSTANT_Methodref:
                info.info.methodref_info.class_index = eat_cp_index();
                info.info.methodref_info.name_and_type_index = eat_cp_index();
                break;
            case CONSTANT_InterfaceMethodref:
                info.info.interface_methodref_info.class_index = eat_cp_index();
                info.info.interface_methodref_info.name_and_type_index = eat_cp_index();
                break;
            case CONSTANT_NameAndType:
                info.info.name_and_type_info.name_index = eat_cp_index();
                info.info.name_and_type_info.descriptor_index = eat_cp_index();
                break;
            case CONSTANT_MethodHandle: {
                min_classfile_format = 51;
                u1 kind = eat_u1();
                if (kind < 1 || kind > 9)
                    throw ParseError("Invalid method handle kind: " + std::to_string(kind));
                info.info.method_handle_info.reference_kind = static_cast<MethodHandleKind>(kind);
                info.info.method_handle_info.reference_index = eat_cp_index();
                break;
            }
            case CONSTANT_MethodType:
                min_classfile_format = 51;
                info.info.method_type_info.descriptor_index = eat_cp_index();
                break;
            case CONSTANT_Dynamic:
                min_classfile_format = 55;
                info.info.dynamic_info.bootstrap_method_attr_index = eat_cp_index();
                info.info.dynamic_info.name_and_type_index = eat_cp_index();
                break;
            case CONSTANT_InvokeDynamic:
                min_classfile_format = 51;
                info.info.invoke_dynamic_info.bootstrap_method_attr_index = eat_cp_index();
                info.info.invoke_dynamic_info.name_and_type_index = eat_cp_index();
                break;
            case CONSTANT_Module:
                min_classfile_format = 53;
                info.info.module_info.name_index = eat_cp_index();
                break;
            case CONSTANT_Package:
                min_classfile_format = 53;
                info.info.package_info.name_index = eat_cp_index();
                break;
            default:
                throw ParseError("Unexpected constant pool tag: " + std::to_string(tag));
        }
        if (min_classfile_format > major_version) {
            throw ParseError("Constant pool entry did not exist in this classfile version");
        }
        result.table.push_back(info);
    }

    return result;
}

std::vector<attribute_info> Parser::parse_attributes() {
    std::vector<attribute_info> result;

    return result;
}