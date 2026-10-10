#pragma once

#include "class_file.hpp"
#include "class_file_reader.hpp"

namespace tjvm {

class ClassFileParser {
public:
    static ClassFile parse(ClassFileReader& reader);

private:
    // simple helper function to parse the constant pool from the class file
    static ConstantPool parse_constant_pool(
        ClassFileReader& reader,
        std::uint16_t count
    );
};

} // namespace tjvm