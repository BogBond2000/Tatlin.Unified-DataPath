#include "../include/FileTape.hpp"

#include <stdexcept>
#include <thread>
#include <filesystem>

namespace fs = std::filesystem;

FileTape::FileTape(const std::string& path, const TapeConfig& config)
    : m_path(path)
    , m_config(config)
{
    m_file.open(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!m_file.is_open()) {
        throw std::runtime_error("FileTape: cannot open file: " + path);
    }

    m_file.seekg(0, std::ios::end);
    size_t bytes = m_file.tellg();
    m_size = bytes / sizeof(int32_t);
    m_file.seekg(0, std::ios::beg);
}

FileTape::FileTape(const std::string& path, size_t size, const TapeConfig& config)
    : m_path(path)
    , m_config(config)
    , m_size(size)
{
    auto parent = fs::path(path).parent_path();
    if (!parent.empty()) fs::create_directories(parent);

    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) {
        throw std::runtime_error("FileTape: cannot create file: " + path);
    }
    int32_t zero = 0;
    for (size_t i = 0; i < size; ++i) {
        ofs.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
    }
    ofs.close();

    m_file.open(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!m_file.is_open()) {
        throw std::runtime_error("FileTape: cannot open file after create: " + path);
    }
}

void FileTape::seekToCurrent() {
    m_file.seekg((m_position * sizeof(int32_t)), std::ios::beg);
    m_file.seekp((m_position * sizeof(int32_t)), std::ios::beg);
}

void FileTape::delay(std::chrono::milliseconds ms) {
    if (ms.count() > 0) {
        std::this_thread::sleep_for(ms);
    }
}

std::optional<int32_t> FileTape::read() {
    if (m_position >= m_size) return std::nullopt;

    delay(m_config.read_delay);

    seekToCurrent();
    int32_t value = 0;
    m_file.read(reinterpret_cast<char*>(&value), sizeof(value));

    if (!m_file) return std::nullopt;
    return value;
}

void FileTape::write(int32_t value) {
    if (m_position >= m_size) {
        throw std::runtime_error("FileTape: write out of bounds");
    }

    delay(m_config.write_delay);

    seekToCurrent();
    m_file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    m_file.flush();
}

bool FileTape::moveForward() {
    delay(m_config.shift_delay);
    if (m_position + 1 < m_size) {
        ++m_position;
        return true;
    }
    if (m_position < m_size) {
        ++m_position;
    }
    return m_position < m_size;
}

bool FileTape::moveBackward() {
    if (m_position == 0) return false;
    delay(m_config.shift_delay);
    --m_position;
    return true;
}

void FileTape::rewind() {
    delay(m_config.rewind_delay);
    m_position = 0;
    seekToCurrent();
}

size_t FileTape::position() const { return m_position; }
size_t FileTape::size() const { return m_size; }
