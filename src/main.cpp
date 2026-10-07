#include <iomanip>
#include <iostream>
#include <string>

#include "tjvm/class_file_reader.hpp"

namespace {

void print_header(const tjvm::ClassFileHeader& header) {
    std::cout << "TJVM Class File Inspector\n";
    std::cout << "=========================\n\n";

    // magic number -> unique identifier for Java class files
    std::cout << "Magic:             0x" << std::hex << std::setw(8) << std::setfill('0') << header.magic << std::dec << '\n';

    // minor version -> indicates the minor version of the Java class file format
    std::cout << "Minor version:     " << header.minor_version << '\n';

    // major version -> indicates the major version of the Java class file format (basically the Java version)
    std::cout << "Major version:     " << header.major_version << '\n';

    // constant pool count -> indicates the number of entries in the constant pool
    std::cout << "Constant pool:     " << header.constant_pool_count << " entries\n";
}

} // namespace

int main(int argc, char* argv[]) {
    // check if the user provided a class file path as an argument
    if (argc != 2) {
        std::cerr << "Usage: tjvm <class-file>\n";
        return 1;
    }

    try {
        // Create a ClassFileReader object to read the class file
        tjvm::ClassFileReader reader(argv[1]);

        // Read the header of the class file
        const auto header = reader.read_header();

        /*
        * magic number is a unique identifier for Java class files.
        * it should always be 0xCAFEBABE for valid Java class files.
        * if the magic number is incorrect, it indicates that the file is not a valid Java class file.
        */
        if (header.magic != 0xCAFEBABE) {
            std::cerr << "Error: file is not a valid Java class file\n";
            return 1;
        }

        // Print the header information
        print_header(header);

    } catch (const std::exception& error) {
        std::cerr << "TJVM error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}