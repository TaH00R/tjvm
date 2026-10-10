#include "tjvm/class_file_reader.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

// Total number of bytes in a Java class file would be 
// 4 (magic) + 2 (minor version) + 2 (major version) + 2 (constant pool count) = 10 bytes

namespace tjvm {

// Reads a Java class file and parses it into a ClassFileHeader
ClassFileReader::ClassFileReader(const std::string& path)
    : file_(path, std::ios::binary) {

    if (!file_) {
        throw std::runtime_error(
            "Failed to open class file: " + path
        );
    }
}

// Reads the header of a Java class file 
std::uint8_t ClassFileReader::read_u1() {
    char byte{};

    /*
    * Read a single byte from the class file,
    */
    if (!file_.read(&byte, sizeof(byte))) {
        throw std::runtime_error("Unexpected end of class file");
    }

    /*
    * Convert the byte to an unsigned 8-bit integer.
    * static_cast ensures that the conversion is explicit and clear.
    */
    return static_cast<std::uint8_t>(
        static_cast<unsigned char>(byte)
    );
}


std::uint16_t ClassFileReader::read_u2() {
    /*
     * Java class files store multi-byte values in big-endian order.
     *
     * For example: 0xCA 0xFE represents the number:
     *   0xCAFE in hexadecimal, which is 51966 in decimal.
     *
     * We build that value manually instead of relying on the machine's native byte order.
     * bcz machine might be little-endian and we want consistency across platforms.
     */
    const auto high = read_u1();
    const auto low = read_u1();

    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8) |
        low
    );
}

std::uint32_t ClassFileReader::read_u4() {
    /*
     * Same idea as read_u2(), but for four bytes.
     *
     * Keeping these primitives here gives the rest of the VM
     * a simple interface for reading the class-file format.
     */
    const auto b1 = read_u1();
    const auto b2 = read_u1();
    const auto b3 = read_u1();
    const auto b4 = read_u1();

    return (static_cast<std::uint32_t>(b1) << 24) |
           (static_cast<std::uint32_t>(b2) << 16) |
           (static_cast<std::uint32_t>(b3) << 8) |
           static_cast<std::uint32_t>(b4);
}

std::uint64_t ClassFileReader::read_u8() {
    /*
     * The class-file format is big-endian, so we read the
     * high and low 32-bit halves separately.
     */
    const auto high = read_u4();
    const auto low = read_u4();

    return (static_cast<std::uint64_t>(high) << 32) | low;
}

// function to read a sequence of bytes from the class file
std::vector<std::uint8_t> ClassFileReader::read_bytes(std::size_t length) {
    std::vector<std::uint8_t> bytes(length);

    for (auto& byte : bytes) {
        byte = read_u1();
    }

    return bytes;
}

ClassFileHeader ClassFileReader::read_header() {
    ClassFileHeader header{};

    header.magic = read_u4();
    header.minor_version = read_u2();
    header.major_version = read_u2();
    header.constant_pool_count = read_u2();

    return header;
}

} // namespace tjvm