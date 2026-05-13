#pragma once

#include <string>
#include <chrono>
#include <fstream>
#include <iostream>

struct TapeConfig {
    std::chrono::milliseconds read_delay{0};
    std::chrono::milliseconds write_delay{0};
    std::chrono::milliseconds shift_delay{0};
    std::chrono::milliseconds rewind_delay{0};
    size_t memory_limit{32};

    static TapeConfig fromFile(const std::string& path) {
        TapeConfig cfg;
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[Config] Cannot open " << path
                      << " — using defaults\n";
            return cfg;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key   = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            try {
                int ms = std::stoi(value);
                if      (key == "read_delay_ms")   cfg.read_delay   = std::chrono::milliseconds(ms);
                else if (key == "write_delay_ms")  cfg.write_delay  = std::chrono::milliseconds(ms);
                else if (key == "shift_delay_ms")  cfg.shift_delay  = std::chrono::milliseconds(ms);
                else if (key == "rewind_delay_ms") cfg.rewind_delay = std::chrono::milliseconds(ms);
                else if (key == "memory_limit_mb") cfg.memory_limit = static_cast<size_t>(ms);
            } catch (...) {
                std::cerr << "[Config] Invalid value for: " << key << "\n";
            }
        }
        return cfg;
    }
};