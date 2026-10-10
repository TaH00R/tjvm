#include <iomanip>
#include <iostream>

#include "tjvm/class_file_parser.hpp"

namespace {

void print_header(const tjvm::ClassFileHeader& header) {
    std::cout << "TJVM Class File Inspector\n";
    std::cout << "=========================\n\n";

    std::cout << "Magic:             0x"
              << std::hex
              << std::setw(8)
              << std::setfill('0')
              << header.magic
              << std::dec
              << '\n';

    std::cout << "Minor version:     "
              << header.minor_version
              << '\n';

    std::cout << "Major version:     "
              << header.major_version
              << '\n';

    std::cout << "Constant pool:     "
              << header.constant_pool_count
              << " slots\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: tjvm <class-file>\n";
        return 1;
    }

    try {
        tjvm::ClassFileReader reader(argv[1]);

        const auto class_file =
            tjvm::ClassFileParser::parse(reader);

        print_header(class_file.header);

        class_file.constant_pool.print();
    } catch (const std::exception& error) {
        std::cerr << "TJVM error: "
                  << error.what()
                  << '\n';

        return 1;
    }

    return 0;
}