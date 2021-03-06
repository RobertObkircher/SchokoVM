#include "parser.hpp"

#include <iterator>
#include <utility>
#include <sstream>
#include <vector>
#include <cstring>
#include "classfile.hpp"

ParseError::ParseError(std::string message) : message(std::move(message)) {}

const char *ParseError::what() const noexcept {
    return message.c_str();
}

Parser::Parser(std::istream &in) : in(in) {
    // TODO How to handle errors?
    // I think the following enables exceptions for all of these.
    // Is this a good idea?
    in.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
}

void Parser::parse(ClassFile *memory) {
    ClassFile &result = *memory;
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
    result.this_class = &check_cp_range_and_type<CONSTANT_Class_info>(result.constant_pool, eat_u2());
    u2 super_class = eat_u2();
    if (super_class == 0) {
        // class Object
        result.super_class_ref = nullptr;
    } else {
        result.super_class_ref = &check_cp_range_and_type<CONSTANT_Class_info>(result.constant_pool, super_class);
    }
    result.super_class = nullptr; // resolved later
    
    result.package_name = std::string_view(result.name()).substr(0, result.name().find_last_of('/'));

    u2 interfaces_count = eat_u2();
    for (int i = 0; i < interfaces_count; ++i)
        result.interfaces.push_back(&check_cp_range_and_type<CONSTANT_Class_info>(result.constant_pool, eat_u2()));

    u2 fields_count = eat_u2();
    for (int i = 0; i < fields_count; ++i) {
        field_info field_info;
        field_info.clazz = memory;
        field_info.access_flags = eat_u2();
        field_info.name_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(result.constant_pool, eat_u2());
        field_info.descriptor_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(result.constant_pool, eat_u2());
        field_info.attributes = parse_attributes(result.constant_pool);
        result.fields.push_back(std::move(field_info));
    }

    u2 methods_count = eat_u2();
    for (int i = 0; i < methods_count; ++i) {
        method_info method_info{};
        method_info.clazz = memory;
        method_info.access_flags = eat_u2();
        method_info.name_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(result.constant_pool, eat_u2());
        method_info.descriptor_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(result.constant_pool, eat_u2());
        method_info.attributes = parse_attributes(result.constant_pool);

        for (auto &attribute : method_info.attributes) {
            Code_attribute *code = std::get_if<Code_attribute>(&attribute.variant);
            if (code) {
                if (method_info.code_attribute)
                    throw ParseError("Method has two code attributes!");
                method_info.code_attribute = code;
            }
        }

        auto &descriptor = method_info.descriptor_index->value;
        if (descriptor.size() < 3)
            throw ParseError("Invalid method descriptor");

        method_info.parameter_count = 0;
        method_info.stack_slots_for_parameters = method_info.is_static() ? 0 : 1;

        MethodDescriptorParts parts{descriptor.c_str()};
        for (; !parts->is_return; ++parts) {
            ++method_info.parameter_count;
            method_info.stack_slots_for_parameters += parts->category;
        }
        method_info.return_category = parts->category;

        if (method_info.name_index->value == "<clinit>") {
            if (result.clinit_index >= 0)
                throw ParseError("Found multiple <clinit> methods");
            if (method_info.descriptor_index->value != "()V")
                throw ParseError("Invalid descriptor");
            if (result.major_version >= 51 && !method_info.is_static())
                throw ParseError("<clinit> must be static");
            result.clinit_index = i;
        }

        result.methods.push_back(std::move(method_info));
    }

    result.attributes = parse_attributes(result.constant_pool);

    in.exceptions(std::ios::goodbit);
    u1 unexpected = eat_u1();
    if (!in.eof())
        throw ParseError("Expected EOF but got " + std::to_string((int) unexpected));
}

std::string Parser::eat_utf8_string(u4 length) {
    std::string str;
    str.resize(length); // not reserve
    in.read(str.data(), length);
    for (size_t i = 0; i < length; ++i) {
        u1 byte = (u1) str[i];
        if (byte == 0 || byte >= 0xf0)
            throw ParseError("Invalid byte in utf8 string: " + std::to_string((int) byte));
    }
    return str;
}

ConstantPool Parser::parse_constant_pool(u2 major_version) {
    ConstantPool result{};

    u2 constant_pool_count = eat_u2();
    result.table.reserve(constant_pool_count);

    result.table.push_back(cp_info{{CONSTANT_Invalid_info{}}});

    auto eat_cp_index = [this, constant_pool_count]() {
        u2 index = eat_u2();
        check_cp_range(index, constant_pool_count);
        return index;
    };

    for (size_t cp_index = 1; cp_index < constant_pool_count; ++cp_index) {
        u1 tag = eat_u1();

        cp_info cpi{};
        u2 min_classfile_format = 45; // 45.3
        switch (tag) {
            case CONSTANT_Utf8: {
                CONSTANT_Utf8_info info;
                u2 length = eat_u2();
                info.value = eat_utf8_string(length);
                cpi.variant = info;
                break;
            }
            case CONSTANT_Integer: {
                CONSTANT_Integer_info info{};
                info.value = eat_s4();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Float: {
                CONSTANT_Float_info info{};
                info.value = eat_float();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Long: {
                CONSTANT_Long_info info{};
                info.value = eat_s8();
                result.table.push_back(cp_info{{info}});
                ++cp_index;
                result.table.push_back(cp_info{{CONSTANT_Invalid_info{}}});
                continue;
            }
            case CONSTANT_Double: {
                CONSTANT_Double_info info{};
                info.value = eat_double();
                result.table.push_back(cp_info{{info}});
                ++cp_index;
                result.table.push_back(cp_info{{CONSTANT_Invalid_info{}}});
                continue;
            }
            case CONSTANT_Class: {
                CONSTANT_Class_info info{};
                info.name_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_String: {
                CONSTANT_String_info info{};
                info.string_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Fieldref: {
                CONSTANT_Fieldref_info info{};
                info.class_index = eat_cp_index();
                info.name_and_type_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Methodref: {
                CONSTANT_Methodref_info info{};
                info.method.class_index = eat_cp_index();
                info.method.name_and_type_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_InterfaceMethodref: {
                CONSTANT_InterfaceMethodref_info info{};
                info.method.class_index = eat_cp_index();
                info.method.name_and_type_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_NameAndType: {
                CONSTANT_NameAndType_info info{};
                info.name_index = eat_cp_index();
                info.descriptor_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_MethodHandle: {
                min_classfile_format = 51;

                CONSTANT_MethodHandle_info info{};
                u1 kind = eat_u1();
                if (kind < 1 || kind > 9)
                    throw ParseError("Invalid method handle kind: " + std::to_string(kind));
                info.reference_kind = static_cast<MethodHandleKind>(kind);
                info.reference_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_MethodType: {
                min_classfile_format = 51;

                CONSTANT_MethodType_info info{};
                info.descriptor_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Dynamic: {
                min_classfile_format = 55;

                CONSTANT_Dynamic_info info{};
                info.bootstrap_method_attr_index = eat_u2();
                highest_parsed_bootstrap_method_attr_index = std::max(highest_parsed_bootstrap_method_attr_index,
                                                                      (int) info.bootstrap_method_attr_index);
                info.name_and_type_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_InvokeDynamic: {
                min_classfile_format = 51;

                CONSTANT_InvokeDynamic_info info{};
                info.bootstrap_method_attr_index = eat_u2();
                highest_parsed_bootstrap_method_attr_index = std::max(highest_parsed_bootstrap_method_attr_index,
                                                                      (int) info.bootstrap_method_attr_index);
                info.name_and_type_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Module: {
                min_classfile_format = 53;

                CONSTANT_Module_info info{};
                info.name_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            case CONSTANT_Package: {
                min_classfile_format = 53;

                CONSTANT_Package_info info{};
                info.name_index = eat_cp_index();
                cpi.variant = info;
                break;
            }
            default:
                throw ParseError("Unexpected constant pool tag: " + std::to_string(tag));
        }
        if (min_classfile_format > major_version) {
            throw ParseError("Constant pool entry did not exist in this classfile version");
        }
        result.table.emplace_back(cpi);
    }

    for (auto &constant : result.table) {
        // TODO maybe use std::visit?
        if (auto it = std::get_if<CONSTANT_Class_info>(&constant.variant)) {
            it->name = &result.get<CONSTANT_Utf8_info>(it->name_index);
        } else if (auto it = std::get_if<CONSTANT_Fieldref_info>(&constant.variant)) {
            it->class_ = &result.get<CONSTANT_Class_info>(it->class_index);
            it->name_and_type = &result.get<CONSTANT_NameAndType_info>(it->name_and_type_index);
        } else if (auto it = std::get_if<CONSTANT_Methodref_info>(&constant.variant)) {
            it->method.class_ = &result.get<CONSTANT_Class_info>(it->method.class_index);
            it->method.name_and_type = &result.get<CONSTANT_NameAndType_info>(it->method.name_and_type_index);
        } else if (auto it = std::get_if<CONSTANT_InterfaceMethodref_info>(&constant.variant)) {
            it->method.class_ = &result.get<CONSTANT_Class_info>(it->method.class_index);
            it->method.name_and_type = &result.get<CONSTANT_NameAndType_info>(it->method.name_and_type_index);
        } else if (auto it = std::get_if<CONSTANT_String_info>(&constant.variant)) {
            it->string = &result.get<CONSTANT_Utf8_info>(it->string_index);
        } else if (auto it = std::get_if<CONSTANT_NameAndType_info>(&constant.variant)) {
            it->name = &result.get<CONSTANT_Utf8_info>(it->name_index);
            it->descriptor = &result.get<CONSTANT_Utf8_info>(it->descriptor_index);
        }
    }

    return result;
}

// TODO we probably do not want to store attributes as an array of structs.
// For example attributes that only go on fields should be stored there as a member variable.
std::vector<attribute_info> Parser::parse_attributes(ConstantPool &constant_pool) {
    std::vector<attribute_info> result;

    u2 attributes_count = eat_u2();

    for (size_t attribute_index = 0; attribute_index < attributes_count; ++attribute_index) {
        attribute_info info;
        info.attribute_name_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(constant_pool, eat_u2());
        info.attribute_length = eat_u4();

        std::string const &s = info.attribute_name_index->value;

        if (s == "ConstantValue") {
            // TODO: sometimes this info must be silently ignored

            if (info.attribute_length != 2) {
                throw ParseError("ConstantValue length must be 2, not " + std::to_string(info.attribute_length));
            }

            ConstantValue_attribute attribute{};
            attribute.constantvalue_index = eat_u2();
            check_cp_range(attribute.constantvalue_index, constant_pool.table.size());
            auto const &variant = constant_pool.table[attribute.constantvalue_index].variant;
            if (std::holds_alternative<CONSTANT_Integer_info>(variant)
                || std::holds_alternative<CONSTANT_Float_info>(variant)
                || std::holds_alternative<CONSTANT_Long_info>(variant)
                || std::holds_alternative<CONSTANT_Double_info>(variant)
                || std::holds_alternative<CONSTANT_String_info>(variant)) {
            } else {
                throw ParseError("Unexpected type for constant pool entry of ConstantValue");
            }
            info.variant = attribute;
        } else if (s == "Code") {
            Code_attribute attribute;
            attribute.max_stack = eat_u2();
            attribute.max_locals = eat_u2();
            u4 code_length = eat_u4();
            attribute.code.resize(code_length);
            in.read(reinterpret_cast<char *>(attribute.code.data()), code_length);

            u2 exception_table_length = eat_u2();
            attribute.exception_table.reserve(exception_table_length);
            for (int i = 0; i < exception_table_length; ++i) {
                ExceptionTableEntry entry{};
                entry.start_pc = eat_u2();
                entry.end_pc = eat_u2();
                entry.handler_pc = eat_u2();
                entry.catch_type = eat_u2();
                attribute.exception_table.push_back(entry);
            }

            attribute.attributes = parse_attributes(constant_pool);

            info.variant = attribute;
        } else if (s == "StackMapTable") {
            // I think/hope this is only used for verification
            for (size_t i = 0; i < info.attribute_length; ++i) eat_u1();
        } else if (s == "Exceptions") {
            Exceptions_attribute attribute;
            u2 number_of_exceptions = eat_u2();
            attribute.exception_index_table.reserve(number_of_exceptions);
            for (size_t i = 0; i < number_of_exceptions; ++i) {
                attribute.exception_index_table.push_back(
                        &check_cp_range_and_type<CONSTANT_Class_info>(constant_pool, eat_u2()));
            }
            info.variant = attribute;
        } else if (s == "Signature") {
            Signature_attribute attribute{};
            attribute.signature_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(constant_pool, eat_u2());
            info.variant = attribute;
        } else if (s == "SourceFile") {
            SourceFile_attribute attribute{};
            attribute.sourcefile_index = &check_cp_range_and_type<CONSTANT_Utf8_info>(constant_pool, eat_u2());
            info.variant = attribute;
        } else if (s == "SourceDebugExtension") {
            SourceDebugExtension_attribute attribute;
            attribute.debug_extension = eat_utf8_string(info.attribute_length);
            info.variant = attribute;
        } else if (s == "LineNumberTable") {
            LineNumberTable_attribute attribute;
            u2 line_number_table_length = eat_u2();
            attribute.line_number_table.reserve(line_number_table_length);
            for (size_t i = 0; i < line_number_table_length; ++i) {
                LineNumberTableEntry entry{};
                entry.start_pc = eat_u2();
                entry.line_number = eat_u2();
                attribute.line_number_table.push_back(entry);
            }
            info.variant = attribute;
        } else if (s == "Deprecated") {
            info.variant = Deprecated_attribute{};
        } else if (s == "BootstrapMethods") {
            BootstrapMethods_attribute attribute;
            u2 num_bootstrap_methods = eat_u2();
            attribute.bootstrap_methods.reserve(num_bootstrap_methods);
            if (highest_parsed_bootstrap_method_attr_index >= num_bootstrap_methods) {
                throw ParseError("Constant pool had an invalid bootstrap method attribute index");
            }
            for (size_t i = 0; i < num_bootstrap_methods; ++i) {
                BootstrapMethod method;
                method.bootstrap_method_ref = &check_cp_range_and_type<CONSTANT_MethodHandle_info>(constant_pool,
                                                                                                   eat_u2());
                u2 num_bootstrap_arguments = eat_u2();
                method.bootstrap_arguments.reserve(num_bootstrap_arguments);
                for (size_t j = 0; j < num_bootstrap_arguments; ++j) {
                    u2 index = eat_u2();
                    check_cp_range(index, constant_pool.table.size());
                    // TODO check loadable
                    method.bootstrap_arguments.push_back(index);
                }
                attribute.bootstrap_methods.push_back(std::move(method));
            }
            info.variant = attribute;
        } else if (s == "MethodParameters") {
            MethodParameters_attribute attribute;
            u1 parameters_count = eat_u1();
            attribute.parameters.reserve(parameters_count);
            for (size_t i = 0; i < parameters_count; ++i) {
                MethodParameter parameter{};
                parameter.name_index = eat_u2(); // 0 or eat_cp_index()
                parameter.access_flags = eat_u2();
                attribute.parameters.push_back(parameter);
            }
            info.variant = attribute;
        } else if (s == "ModuleMainClass") {
            ModuleMainClass_attribute attribute{};
            attribute.main_class_index = eat_u2();
            info.variant = attribute;
        } else if (s == "NestHost") {
            NestHost_attribute attribute{};
            attribute.host_class_index = &check_cp_range_and_type<CONSTANT_Class_info>(constant_pool, eat_u2());
            info.variant = attribute;
        } else if (s == "NestMembers") {
            NestMembers_attribute attribute;
            u2 number_of_classes = eat_u2();
            attribute.classes.reserve(number_of_classes);
            for (size_t i = 0; i < number_of_classes; ++i) {
                attribute.classes.push_back(&check_cp_range_and_type<CONSTANT_Class_info>(constant_pool, eat_u2()));
            }
            info.variant = attribute;
        } else {
            for (size_t i = 0; i < info.attribute_length; ++i) {
                eat_u1();
            }
            continue;
        }
        result.push_back(std::move(info));
    }

    return result;
}

MethodDescriptorParts::MethodDescriptorParts(const char *ptr) : m_start(ptr), m_length(0), m_part() {
    if (m_start[0] != '(') {
        throw ParseError("Descriptor must start with '('");
    }
    ++m_start;
    token();
    m_part.type_name = {m_start, m_length};
}

MethodDescriptorParts &MethodDescriptorParts::operator++() {
    m_start += m_length;
    m_length = 0;
    m_part = {};
    token();
    m_part.type_name = {m_start, m_length};
    return *this;
}

void MethodDescriptorParts::token() {
    switch (m_start[m_length]) {
        case '[': {
            ++m_length; // skip '['
            token();
            m_part.category = 1;
            ++m_part.array_dimensions;
            return;
        }
        case 'L': {
            ++m_length; // skip 'L'
            while (m_start[m_length] && m_start[m_length] != ';') {
                ++m_length;
            }
            if (!m_start[m_length]) {
                throw ParseError("");
            }
            ++m_length; // skip ';'
            m_part.category = 1;
            return;
        }
        case 'D':
        case 'J': {
            ++m_length;
            m_part.category = 2;
            return;
        }
        case 'B':
        case 'C':
        case 'F':
        case 'I':
        case 'S':
        case 'Z': {
            ++m_length;
            m_part.category = 1;
            return;
        }
        case ')': {
            if (m_length != 0) {
                throw ParseError("Unexpected ')'");
            }
            ++m_start; // increase pointer instead of length

            if (m_part.is_return) {
                throw ParseError("Found more than one ')'");
            }
            m_part.is_return = true;

            if (m_start[m_length] == 'V') {
                ++m_length;
                m_part.category = 0;
            } else {
                token();
            }
            if (m_start[m_length] != 0) {
                throw ParseError("Expected end");
            }
            return;
        }
        case '\0': {
            throw ParseError("Unexpected end of descriptor");
        }
        default: {
            throw ParseError("Unexpected character");
        }
    }
}

