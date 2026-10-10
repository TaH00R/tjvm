#include "tjvm/class_file_parser.hpp"

#include <bit>
#include <stdexcept>


//Class Parser is responsible for parsing a Java class file and converting it into a ClassFile structure.
namespace tjvm {

namespace {
ConstantTag read_tag(ClassFileReader& reader) {
    const auto raw_tag = reader.read_u1();

    switch (raw_tag) {
        case 1:
            return ConstantTag::Utf8;
        case 3:
            return ConstantTag::Integer;
        case 4:
            return ConstantTag::Float;
        case 5:
            return ConstantTag::Long;
        case 6:
            return ConstantTag::Double;
        case 7:
            return ConstantTag::Class;
        case 8:
            return ConstantTag::String;
        case 9:
            return ConstantTag::Fieldref;
        case 10:
            return ConstantTag::Methodref;
        case 11:
            return ConstantTag::InterfaceMethodref;
        case 12:
            return ConstantTag::NameAndType;
        case 15:
            return ConstantTag::MethodHandle;
        case 16:
            return ConstantTag::MethodType;
        case 17:
            return ConstantTag::Dynamic;
        case 18:
            return ConstantTag::InvokeDynamic;
        case 19:
            return ConstantTag::Module;
        case 20:
            return ConstantTag::Package;
        default:
            throw std::runtime_error(
                "Unknown constant pool tag: " +
                std::to_string(raw_tag)
            );
    }
}

} // namespace

ClassFile ClassFileParser::parse(
    ClassFileReader& reader
) {
    ClassFile class_file{
        .header = reader.read_header(),
        .constant_pool = ConstantPool(
            class_file.header.constant_pool_count
        )
    };

    if (class_file.header.magic != 0xCAFEBABE) {
        throw std::runtime_error(
            "Invalid class file magic"
        );
    }

    /*
     * The header tells us how many slots are present in
     * the constant pool. We parse those slots next.
     */
    class_file.constant_pool = parse_constant_pool(
        reader,
        class_file.header.constant_pool_count
    );

    return class_file;
}

ConstantPool ClassFileParser::parse_constant_pool(
    ClassFileReader& reader,
    std::uint16_t count
) {
    ConstantPool pool(count);

    for (std::size_t index = 1; index < count; ++index) {
        const auto tag = read_tag(reader);

        switch (tag) {
            case ConstantTag::Utf8: {
                const auto length = reader.read_u2();
                const auto bytes = reader.read_bytes(length);

                std::string value(
                    bytes.begin(),
                    bytes.end()
                );

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
                        .value = std::bit_cast<std::int32_t>(
                            bits
                        )
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
                        .value = std::bit_cast<std::int64_t>(
                            bits
                        )
                    }
                });

                /*
                 * Long and Double values occupy two entries.
                 * The following slot is therefore reserved.
                 */
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
                        .name_and_type_index =
                            reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::Methodref: {
                pool.set(index, {
                    tag,
                    ConstantMethodref{
                        .class_index = reader.read_u2(),
                        .name_and_type_index =
                            reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::InterfaceMethodref: {
                pool.set(index, {
                    tag,
                    ConstantInterfaceMethodref{
                        .class_index = reader.read_u2(),
                        .name_and_type_index =
                            reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::NameAndType: {
                pool.set(index, {
                    tag,
                    ConstantNameAndType{
                        .name_index = reader.read_u2(),
                        .descriptor_index =
                            reader.read_u2()
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
                        .bootstrap_method_attr_index =
                            reader.read_u2(),
                        .name_and_type_index =
                            reader.read_u2()
                    }
                });

                break;
            }

            case ConstantTag::InvokeDynamic: {
                pool.set(index, {
                    tag,
                    ConstantInvokeDynamic{
                        .bootstrap_method_attr_index =
                            reader.read_u2(),
                        .name_and_type_index =
                            reader.read_u2()
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