#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace tjvm {
/*
* lemme explain what this thing basically is!
* so these tags are used to identify the type of constant
* in the constant pool of a Java class file.
* each tag corresponds to a specific type of constant, like Utf8 strings, integers, floats, classes, methods, etc.
* the size of each constant is determined by its tag, and the data for each constant is stored in a specific format.
* for example, a Utf8 constant has a tag of 1 and is followed by 
* a length field and the actual string data, 
* while an Integer constant has a tag of 3 and is followed by a 4-byte integer value.
* these tags are predefined in the JVM specification
 */

 // me 3 days later -> thanks for the comments cz i forgot what the hell these tags are for
enum class ConstantTag : std::uint8_t {
    Utf8 = 1,
    Integer = 3,
    Float = 4,
    Long = 5,
    Double = 6,
    Class = 7,
    String = 8,
    Fieldref = 9,
    Methodref = 10,
    InterfaceMethodref = 11,
    NameAndType = 12,
    MethodHandle = 15,
    MethodType = 16,
    Dynamic = 17,
    InvokeDynamic = 18,
    Module = 19,
    Package = 20
};

struct ConstantUtf8 {
    std::string value;
};

struct ConstantInteger {
    std::int32_t value;
};

struct ConstantFloat {
    float value;
};

struct ConstantLong {
    std::int64_t value;
};

struct ConstantDouble {
    double value;
};

struct ConstantClass {
    std::uint16_t name_index;
};

struct ConstantString {
    std::uint16_t string_index;
};

struct ConstantFieldref {
    std::uint16_t class_index;
    std::uint16_t name_and_type_index;
};

struct ConstantMethodref {
    std::uint16_t class_index;
    std::uint16_t name_and_type_index;
};

struct ConstantInterfaceMethodref {
    std::uint16_t class_index;
    std::uint16_t name_and_type_index;
};

struct ConstantNameAndType {
    std::uint16_t name_index;
    std::uint16_t descriptor_index;
};

struct ConstantMethodHandle {
    std::uint8_t reference_kind;
    std::uint16_t reference_index;
};

struct ConstantMethodType {
    std::uint16_t descriptor_index;
};

struct ConstantDynamic {
    std::uint16_t bootstrap_method_attr_index;
    std::uint16_t name_and_type_index;
};

struct ConstantInvokeDynamic {
    std::uint16_t bootstrap_method_attr_index;
    std::uint16_t name_and_type_index;
};

struct ConstantModule {
    std::uint16_t name_index;
};

struct ConstantPackage {
    std::uint16_t name_index;
};

using ConstantInfo = std::variant<
    ConstantUtf8,
    ConstantInteger,
    ConstantFloat,
    ConstantLong,
    ConstantDouble,
    ConstantClass,
    ConstantString,
    ConstantFieldref,
    ConstantMethodref,
    ConstantInterfaceMethodref,
    ConstantNameAndType,
    ConstantMethodHandle,
    ConstantMethodType,
    ConstantDynamic,
    ConstantInvokeDynamic,
    ConstantModule,
    ConstantPackage
>;

struct ConstantPoolEntry {
    ConstantTag tag;
    ConstantInfo info;
};

class ConstantPool {
public:
    explicit ConstantPool(std::uint16_t count);

    void set(
        std::size_t index,
        ConstantPoolEntry entry
    );

    const std::optional<ConstantPoolEntry>& at(std::size_t index) const;

    std::size_t size() const;

    void print() const;

private:
    std::vector<std::optional<ConstantPoolEntry>> entries_;
};

} // namespace tjvm