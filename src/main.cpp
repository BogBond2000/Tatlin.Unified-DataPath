#include "../include/FileTape.hpp"
#include "../include/TapeSorter.hpp"
#include "../include/TapeConfig.hpp"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: tape_sorter <input_file> <output_file> "
                     "[config_file]\n";
        return 1;
    }

    const std::string input_path  = argv[1];
    const std::string output_path = argv[2];
    const std::string config_path = (argc >= 4) ? argv[3] : "config.ini";

    TapeConfig config = TapeConfig::fromFile(config_path);

    std::cout << "[Config] read_delay   = "
              << config.read_delay.count()   << " ms\n";
    std::cout << "[Config] write_delay  = "
              << config.write_delay.count()  << " ms\n";
    std::cout << "[Config] shift_delay  = "
              << config.shift_delay.count()  << " ms\n";
    std::cout << "[Config] rewind_delay = "
              << config.rewind_delay.count() << " ms\n";
    std::cout << "[Config] memory_limit = "
              << config.memory_limit         << " MB\n";

    FileTape input(input_path, config);
    std::cout << "[Main] Input tape: "
              << input.size() << " elements\n";

    FileTape output(output_path, input.size(), config);

    TapeSorter sorter("tmp", config);
    sorter.sort(input, output);

    std::cout << "[Main] Done. Result written to " << output_path << "\n";
    return 0;
}