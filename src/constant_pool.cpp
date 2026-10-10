#include "tjvm/constant_pool.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace tjvm {

ConstantPool::ConstantPool(std::uint16_t count)
    : entries_(count) {
}

// Sets a constant pool entry at the specified index
void ConstantPool::set(
    std::size_t index,
    ConstantPoolEntry entry
) {
    if (index == 0 || index >= entries_.size()) {
        throw std::out_of_range(
            "Invalid constant pool index"
        );
    }

    entries_[index] = std::move(entry);
}


// Retrieves a constant pool entry at the specified index
const std::optional<ConstantPoolEntry>& ConstantPool::at(std::size_t index) const {
    if (index >= entries_.size()) {
        throw std::out_of_range(
            "Invalid constant pool index"
        );
    }

    return entries_[index];
}

std::size_t ConstantPool::size() const {
    return entries_.size();
}


/*
*Basically a repititve function
* it jst prints the constant pool entries in a human-readable format.
* It iterates through the entries in the constant pool and prints their index, type, and
* associated data based on the type of constant. (easier said than done!)
*/
void ConstantPool::print() const {
    std::cout << "\nConstant Pool\n";
    std::cout << "=============\n";

    for (std::size_t i = 1; i < entries_.size(); ++i) {
        const auto& entry = entries_[i];

        /*
         * Long and Double constants occupy two constant-pool
         * slots. The second slot is intentionally left empty.
         */
        if (!entry.has_value()) {
            continue;
        }

        std::cout << "#"
                  << std::setw(3)
                  << i
                  << " ";

        std::visit(
            [](const auto& constant) {
                using T = std::decay_t<decltype(constant)>;

                if constexpr (
                    std::is_same_v<T, ConstantUtf8>
                ) {
                    std::cout
                        << "Utf8               "
                        << constant.value;
                } else if constexpr (
                    std::is_same_v<T, ConstantInteger>
                ) {
                    std::cout
                        << "Integer            "
                        << constant.value;
                } else if constexpr (
                    std::is_same_v<T, ConstantFloat>
                ) {
                    std::cout
                        << "Float              "
                        << constant.value;
                } else if constexpr (
                    std::is_same_v<T, ConstantLong>
                ) {
                    std::cout
                        << "Long               "
                        << constant.value;
                } else if constexpr (
                    std::is_same_v<T, ConstantDouble>
                ) {
                    std::cout
                        << "Double             "
                        << constant.value;
                } else if constexpr (
                    std::is_same_v<T, ConstantClass>
                ) {
                    std::cout
                        << "Class              #"
                        << constant.name_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantString>
                ) {
                    std::cout
                        << "String             #"
                        << constant.string_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantFieldref>
                ) {
                    std::cout
                        << "Fieldref           #"
                        << constant.class_index
                        << ".#"
                        << constant.name_and_type_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantMethodref>
                ) {
                    std::cout
                        << "Methodref          #"
                        << constant.class_index
                        << ".#"
                        << constant.name_and_type_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantInterfaceMethodref>
                ) {
                    std::cout
                        << "InterfaceMethodref #"
                        << constant.class_index
                        << ".#"
                        << constant.name_and_type_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantNameAndType>
                ) {
                    std::cout
                        << "NameAndType        #"
                        << constant.name_index
                        << ":#"
                        << constant.descriptor_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantMethodHandle>
                ) {
                    std::cout
                        << "MethodHandle       kind="
                        << static_cast<int>(
                            constant.reference_kind
                        )
                        << " ref=#"
                        << constant.reference_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantMethodType>
                ) {
                    std::cout
                        << "MethodType         #"
                        << constant.descriptor_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantDynamic>
                ) {
                    std::cout
                        << "Dynamic            bootstrap=#"
                        << constant.bootstrap_method_attr_index
                        << " name_type=#"
                        << constant.name_and_type_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantInvokeDynamic>
                ) {
                    std::cout
                        << "InvokeDynamic      bootstrap=#"
                        << constant.bootstrap_method_attr_index
                        << " name_type=#"
                        << constant.name_and_type_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantModule>
                ) {
                    std::cout
                        << "Module             #"
                        << constant.name_index;
                } else if constexpr (
                    std::is_same_v<T, ConstantPackage>
                ) {
                    std::cout
                        << "Package            #"
                        << constant.name_index;
                }
            },
            entry->info
        );

        std::cout << '\n';
    }
}

} // namespace tjvm