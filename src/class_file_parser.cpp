
#include "tjvm/class_file_parser.hpp"

#include <bit>
#include <stdexcept>
#include <string>
#include <utility>

namespace tjvm {

namespace {

// Every constant-pool entry starts with a tag that tells us how
// to interpret the bytes that follow it.
ConstantTag read_tag(ClassFileReader& reader) {
    const auto raw_tag = reader.read_u1();

    switch (raw_tag) {
        case 1:  return ConstantTag::Utf8;
        case 3:  return ConstantTag::Integer;
        case 4:  return ConstantTag::Float;
        case 5:  return ConstantTag::Long;
        case 6:  return ConstantTag::Double;
        case 7:  return ConstantTag::Class;
        case 8:  return ConstantTag::String;
        case 9:  return ConstantTag::Fieldref;
        case 10: return ConstantTag::Methodref;
        case 11: return ConstantTag::InterfaceMethodref;
        case 12: return ConstantTag::NameAndType;
        case 15: return ConstantTag::MethodHandle;
        case 16: return ConstantTag::MethodType;
        case 17: return ConstantTag::Dynamic;
        case 18: return ConstantTag::InvokeDynamic;
        case 19: return ConstantTag::Module;
        case 20: return ConstantTag::Package;

        default:
            throw std::runtime_error(
                "Unknown constant pool tag: " +
                std::to_string(raw_tag)
            );
    }
}

} // namespace

ClassFile ClassFileParser::parse(ClassFileReader& reader) {
    // Read the header first. Its constant-pool count determines
    // how many entries we need to read next.
    const auto header = reader.read_header();

    if (header.magic != 0xCAFEBABE) {
        throw std::runtime_error("Invalid class file magic");
    }

    // The constant pool comes immediately after the header.
    // Once it is parsed, we can use its entries to interpret
    // the class names, member names and other references.
    ClassFile class_file{
    .header = header,
    .constant_pool = parse_constant_pool(
        reader,
        header.constant_pool_count
    ),
    .access_flags = 0,
    .this_class = 0,
    .super_class = 0,
    .interfaces = {},
    .fields = {},
    .methods = {},
    .attributes = {}
};

    // These fields describe the class itself. They contain
    // constant-pool indices rather than actual names.
    class_file.access_flags = reader.read_u2();
    class_file.this_class = reader.read_u2();
    class_file.super_class = reader.read_u2();

    // Read the interfaces implemented by this class.
    const auto interface_count = reader.read_u2();
    class_file.interfaces.reserve(interface_count);

    for (std::uint16_t i = 0; i < interface_count; ++i) {
        class_file.interfaces.push_back(reader.read_u2());
    }

    // Fields and methods share the same binary structure:
    // access flags, name, descriptor and a list of attributes.
    const auto field_count = reader.read_u2();
    class_file.fields.reserve(field_count);

    for (std::uint16_t i = 0; i < field_count; ++i) {
        class_file.fields.push_back(parse_member(reader));
    }

    const auto method_count = reader.read_u2();
    class_file.methods.reserve(method_count);

    for (std::uint16_t i = 0; i < method_count; ++i) {
        class_file.methods.push_back(parse_member(reader));
    }

    // Class-level attributes come last. Examples include
    // SourceFile, InnerClasses and BootstrapMethods.
    class_file.attributes = parse_attributes(reader);

    return class_file;
}

std::vector<AttributeInfo> ClassFileParser::parse_attributes(
    ClassFileReader& reader
) {
    const auto count = reader.read_u2();

    std::vector<AttributeInfo> attributes;
    attributes.reserve(count);

    for (std::uint16_t i = 0; i < count; ++i) {
        const auto name_index = reader.read_u2();
        const auto length = reader.read_u4();

        // Attribute formats vary, so preserve their payloads
        // as bytes until we implement individual attribute parsers.
        auto info = reader.read_bytes(length);

        attributes.push_back({
            .name_index = name_index,
            .info = std::move(info)
        });
    }

    return attributes;
}

MemberInfo ClassFileParser::parse_member(ClassFileReader& reader) {
    MemberInfo member{
        .access_flags = reader.read_u2(),
        .name_index = reader.read_u2(),
        .descriptor_index = reader.read_u2(),
        .attributes = parse_attributes(reader)
    };

    return member;
}

ConstantPool ClassFileParser::parse_constant_pool(
    ClassFileReader& reader,
    std::uint16_t count
) {
    ConstantPool pool(count);

    // Index zero is reserved by the class-file format, so
    // actual constant-pool entries start at index one.
    for (std::size_t index = 1; index < count; ++index) {
        const auto tag = read_tag(reader);

        switch (tag) {
            case ConstantTag::Utf8: {
                const auto length = reader.read_u2();
                const auto bytes = reader.read_bytes(length);

                std::string value(bytes.begin(), bytes.end());

                pool.set(index, {
                    tag,
                    ConstantUtf8{
                        .value = std::move(value)
                    }
                });

                break;
            }

            case ConstantTag::Integer: {
                const auto bits = reader.read_u4();

                pool.set(index, {
                    tag,
                    ConstantInteger{
                        .value = std::bit_cast<std::int32_t>(bits)
                    }
                });

                break;
            }

            case ConstantTag::Float: {
                const auto bits = reader.read_u4();

                pool.set(index, {
                    tag,
                    ConstantFloat{
                        .value = std::bit_cast<float>(bits)
                    }
                });

                break;
            }

            case ConstantTag::Long: {
                const auto bits = reader.read_u8();

                pool.set(index, {
                    tag,
                    ConstantLong{
                        .value = std::bit_cast<std::int64_t>(bits)
                    }
                });

                // Long and Double entries occupy two constant-pool
                // slots. The next slot is reserved and has no entry.
                ++index;

                if (index >= count) {
                    throw std::runtime_error(
                        "Long constant extends beyond pool"
                    );
                }

                break;
            }

            case ConstantTag::Double: {
                const auto bits = reader.read_u8();

                pool.set(index, {
                    tag,
                    ConstantDouble{
                        .value = std::bit_cast<double>(bits)
                    }
                });

                // Reserve the second slot used by a Double entry.
                ++index;

                if (index >= count) {
                    throw std::runtime_error(
                        "Double constant extends beyond pool"
                    );
                }

                break;
            }

            case ConstantTag::Class: {
                pool.set(index, {
                    tag,
                    ConstantClass{
                        .name_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::String: {
                pool.set(index, {
                    tag,
                    ConstantString{
                        .string_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::Fieldref: {
                pool.set(index, {
                    tag,
                    ConstantFieldref{
                        .class_index = reader.read_u2(),
                        .name_and_type_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::Methodref: {
                pool.set(index, {
                    tag,
                    ConstantMethodref{
                        .class_index = reader.read_u2(),
                        .name_and_type_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::InterfaceMethodref: {
                pool.set(index, {
                    tag,
                    ConstantInterfaceMethodref{
                        .class_index = reader.read_u2(),
                        .name_and_type_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::NameAndType: {
                pool.set(index, {
                    tag,
                    ConstantNameAndType{
                        .name_index = reader.read_u2(),
                        .descriptor_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::MethodHandle: {
                pool.set(index, {
                    tag,
                    ConstantMethodHandle{
                        .reference_kind = reader.read_u1(),
                        .reference_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::MethodType: {
                pool.set(index, {
                    tag,
                    ConstantMethodType{
                        .descriptor_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::Dynamic: {
                pool.set(index, {
                    tag,
                    ConstantDynamic{
                        .bootstrap_method_attr_index = reader.read_u2(),
                        .name_and_type_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::InvokeDynamic: {
                pool.set(index, {
                    tag,
                    ConstantInvokeDynamic{
                        .bootstrap_method_attr_index = reader.read_u2(),
                        .name_and_type_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::Module: {
                pool.set(index, {
                    tag,
                    ConstantModule{
                        .name_index = reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::Package: {
                pool.set(index, {
                    tag,
                    ConstantPackage{
                        .name_index = reader.read_u2()
                    }
                });

                break;
            }
        }
    }

    return pool;
}

} // namespace tjvm
