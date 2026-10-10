#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "tjvm/class_file.hpp"

namespace tjvm {

// ts is responsible for reading a Java class and parsing it into a ClassFileHeader
class ClassFileReader {
public:
    explicit ClassFileReader(const std::string& path); 
    ClassFileHeader read_header(); // function to read the class file header

    std::uint8_t read_u1(); // read 1 byte 
    std::uint16_t read_u2(); // read 2 bytes
    std::uint32_t read_u4(); // read 4 bytes 
    std::uint64_t read_u8(); // read 8 bytes

    std::vector<std::uint8_t> read_bytes(std::size_t length);

private:
    std::ifstream file_;
};

} // namespace tjvm