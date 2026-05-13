#pragma once

#include "ITape.hpp"
#include "TapeConfig.hpp"

#include <string>
#include <vector>

class TapeSorter final {
public:
    TapeSorter(const std::string& tmp_dir, const TapeConfig& config);

    TapeSorter(const TapeSorter&) = delete;
    TapeSorter& operator=(const TapeSorter&) = delete;

    void sort(ITape& input, ITape& output);

private:
    std::string m_tmp_dir;
    TapeConfig  m_config;

    std::vector<std::string> createRuns(ITape& input);
    void mergeRuns(const std::vector<std::string>& run_paths, ITape& output);
};