#ifndef SCHOKOVM_PARSER_HPP
#define SCHOKOVM_PARSER_HPP

#include <fstream>
#include <iostream>
#include <optional>

#include "classfile.hpp"
#include "future.hpp"

struct ParseError : std::exception {
    std::string message;

    explicit ParseError(std::string message);

    [[nodiscard]] const char *what() const noexcept override;
};

inline void check_cp_range(u2 index, size_t size) {
    if (index == 0 || index >= size)
        throw ParseError("Constant pool index out of range");
}

template<class T>
inline T &check_cp_range_and_type(ConstantPool &pool, u2 index) {
    check_cp_range(index, pool.table.size());
    if (!std::holds_alternative<T>(pool.table[index].variant)) {
        throw ParseError("Unexpected constant pool type");
    }
    return pool.get<T>(index);
}

class Parser {
    std::istream &in;
    int highest_parsed_bootstrap_method_attr_index = -1;

    inline u1 eat_u1() {
        // WARNING: >> and .get() are intended for text. They would filter out 0x0A characters...
        u1 a;
        in.read((char *) &a, 1);
        return a;
    }

    inline u2 eat_u2() {
        u1 bytes[2];
        in.read((char *) bytes, 2);
        return (u2) ((bytes[0] << 8) | (bytes[1] << 0));
    }

    inline u4 eat_u4() {
        // NOTE: This is u1 because casting negative chars to u4 would cause problems with sign extension.
        // Correct cast: (u4)(unsigned char)(char) 'x'
        u1 bytes[4];
        in.read((char *) bytes, 4);
        return ((u4) bytes[0] << 24) | ((u4) bytes[1] << 16) | ((u4) bytes[2] << 8) | ((u4) bytes[3] << 0);
    }

    inline u8 eat_u8() {
        u1 bytes[8];
        in.read((char *) bytes, 8);
        return ((u8) bytes[0] << 56) | ((u8) bytes[1] << 48) | ((u8) bytes[2] << 40) | ((u8) bytes[3] << 32) |
               ((u8) bytes[4] << 24) | ((u8) bytes[5] << 16) | ((u8) bytes[6] << 8) | ((u8) bytes[7] << 0);
    }

    inline s4 eat_s4() { return future::bit_cast<s4>(eat_u4()); }

    inline s8 eat_s8() { return future::bit_cast<s8>(eat_u8()); }

    inline float eat_float() { return future::bit_cast<float>(eat_u4()); }

    inline double eat_double() { return future::bit_cast<double>(eat_u8()); }

public:
    explicit Parser(std::istream &in);

    ClassFile parse();

    ConstantPool parse_constant_pool(u2 major_version);

    std::string eat_utf8_string(u4 length);

    std::vector<attribute_info> parse_attributes(ConstantPool &constant_pool);
};

struct DescriptorPart {
    std::string_view type_name{}; // examples:   I   [[I   Ljava.lang.String;
    size_t array_dimensions = 0; // how many [
    u1 category = 0; // void=0, category 1, category 2
    bool is_return = false;
};

struct MethodDescriptorParts : std::iterator<std::forward_iterator_tag, DescriptorPart> {
    explicit MethodDescriptorParts(char const *ptr);

    reference operator*() { return m_part; }
    pointer operator->() { return &m_part; }

    MethodDescriptorParts &operator++();

    bool operator==(const MethodDescriptorParts& other) { return m_start == other.m_start; };
    bool operator!=(const MethodDescriptorParts& other) { return m_start != other.m_start; };

private:
    char const  *m_start;
    size_t m_length;
    DescriptorPart m_part;

    void token();
};

#endif //SCHOKOVM_PARSER_HPP
