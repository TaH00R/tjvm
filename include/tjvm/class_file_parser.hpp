#pragma once

#include "tjvm/class_file.hpp"
#include "tjvm/class_file_reader.hpp"

namespace tjvm {

class ClassFileParser {
public:
    // parse the class file from the given reader and return a ClassFile object
    static ClassFile parse(ClassFileReader& reader);

private:
    // simple helper function to parse the constant pool from the class file
    static ConstantPool parse_constant_pool(
        ClassFileReader& reader,
        std::uint16_t count
    );

    // simple helper function to parse the 
    static std::vector<AttributeInfo> parse_attributes(
        ClassFileReader& reader
    );


    // simple helper function to parse the class file interfaces from the class file
    static MemberInfo parse_member(
        ClassFileReader& reader
    );
};

} // namespace tjvm