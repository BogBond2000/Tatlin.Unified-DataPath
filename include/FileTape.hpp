#pragma once

#include "ITape.hpp"
#include "TapeConfig.hpp"

#include <string>
#include <fstream>

class FileTape final : public ITape {
public:
    FileTape(const std::string& path, const TapeConfig& config);

    FileTape(const std::string& path, size_t size, const TapeConfig& config);

    ~FileTape() override = default;
    FileTape(const FileTape&) = delete;
    FileTape& operator=(const FileTape&) = delete;

    std::optional<int32_t> read() override;
    void write(int32_t) override;
    bool moveForward()  override;
    bool moveBackward() override;
    void rewind() override;
    size_t position() const override;
    size_t size() const override;

private:
    std::string  m_path;
    std::fstream m_file;
    TapeConfig   m_config;
    size_t       m_position{0};
    size_t       m_size{0};

    void seekToCurrent();
    static void delay(std::chrono::milliseconds ms);
};
