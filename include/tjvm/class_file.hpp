#pragma once

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

} // namespace tjvm