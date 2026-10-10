#pragma once

#include <tjvm/constant_pool.hpp>
#include <cstdint>


namespace tjvm {

/*
 * the first piece of information stored in every
 * Java class file.
 *
 * As the class-file parser grows, will eventually include
 * the constant pool, fields, methods, and attributes. (i hope)
 */

// basically stores the Java class file header information
struct ClassFileHeader {
    std::uint32_t magic;
    std::uint16_t minor_version;
    std::uint16_t major_version;
    std::uint16_t constant_pool_count;
};

//Java class file attribute information
struct AttributeInfo {
    std::uint16_t name_index; // index into the constant pool
    std:: vector<std::uint8_t> info; // the actual attribute data
};

//Java class member information (fields and methods)
struct MemberInfo {
    std::uint16_t access_flags; // access flags for the member
    std::uint16_t name_index; // index into the constant pool for the member's name
    std::uint16_t descriptor_index; // index into the constant pool for the member's descriptor
    std::vector<AttributeInfo> attributes; // attributes associated with the member
};

struct ClassFile {
    ClassFileHeader header;
    ConstantPool constant_pool;

    std::uint16_t access_flags;
    std::uint16_t this_class;
    std::uint16_t super_class;

    std::vector<std::uint16_t> interfaces;
    std::vector<MemberInfo> fields;
    std::vector<MemberInfo> methods;
    std::vector<AttributeInfo> attributes;
};

} // namespace tjvm