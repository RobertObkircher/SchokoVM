#ifndef SCHOKOVM_PARSER_HPP
#define SCHOKOVM_PARSER_HPP

#include "fstream"
#include "iostream"
#include "optional"

#include "classfile.hpp"

struct ParseError : std::exception {
    std::string message;

    explicit ParseError(std::string message);

    [[nodiscard]] const char *what() const noexcept override;
};

class Parser {
    std::istream &in;
    u2 constant_pool_count;

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

    inline s1 eat_s1() { return (s1) eat_u1(); }

    inline s2 eat_s2() { return (s2) eat_u2(); }

    inline s4 eat_s4() { return (s4) eat_u4(); }

    inline s8 eat_s8() { return (s8) eat_u8(); }

public:
    explicit Parser(std::istream &in);

    ClassFile parse();

    ConstantPool parse_constant_pool(u2 major_version);

    u2 eat_cp_index();

    std::vector<attribute_info> parse_attributes(const ConstantPool &constant_pool);
};

#endif //SCHOKOVM_PARSER_HPP
