#include "../include/TapeSorter.hpp"
#include "../include/FileTape.hpp"

#include <algorithm>
#include <vector>
#include <queue>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

TapeSorter::TapeSorter(const std::string& tmp_dir, const TapeConfig& config)
    : m_tmp_dir(tmp_dir)
    , m_config(config)
{
    fs::create_directories(tmp_dir);
}

void TapeSorter::sort(ITape& input, ITape& output) {
    input.rewind();

    auto run_paths = createRuns(input);
    std::cout << "[Sorter] Created " << run_paths.size() << " runs\n";

    mergeRuns(run_paths, output);
    std::cout << "[Sorter] Merge complete\n";

    for (const auto& p : run_paths) {
        fs::remove(p);
    }
}

std::vector<std::string> TapeSorter::createRuns(ITape& input) {
    const size_t max_elements =
        (m_config.memory_limit * 1024 * 1024) / sizeof(int32_t);
    if (max_elements == 0) {
        throw std::runtime_error("Memory limit too small to hold even one int32_t");
    }

    std::vector<std::string> run_paths;
    std::vector<int32_t>     buffer;
    buffer.reserve(max_elements);

    int    run_index  = 0;
    bool   exhausted  = false;

    while (!exhausted) {
        buffer.clear();

        auto first = input.read();
        if (!first.has_value()) break;
        buffer.push_back(*first);

        for (size_t i = 1; i < max_elements; ++i) {
            if (!input.moveForward()) {
                exhausted = true;
                break;
            }
            auto val = input.read();
            if (!val.has_value()) {
                exhausted = true;
                break;
            }
            buffer.push_back(*val);
        }

        if (buffer.empty()) break;

        std::sort(buffer.begin(), buffer.end());

        std::string run_path = m_tmp_dir + "/run_"
                               + std::to_string(run_index) + ".bin";

        FileTape run_tape(run_path, buffer.size(), m_config);
        for (size_t i = 0; i < buffer.size(); ++i) {
            run_tape.write(buffer[i]);
            if (i + 1 < buffer.size()) run_tape.moveForward();
        }

        run_paths.push_back(run_path);
        std::cout << "[Sorter] Run " << (run_index + 1)
                  << " written (" << buffer.size() << " elements)\n";
        ++run_index;

        if (!exhausted) {
            if (!input.moveForward()) {
                exhausted = true;
            }
        }
    }

    return run_paths;
}

void TapeSorter::mergeRuns(const std::vector<std::string>& run_paths,
                            ITape& output)
{
    std::vector<std::unique_ptr<FileTape>> tapes;
    tapes.reserve(run_paths.size());
    for (const auto& p : run_paths) {
        tapes.push_back(std::make_unique<FileTape>(p, m_config));
        tapes.back()->rewind();
    }

    using Entry = std::pair<int32_t, size_t>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> heap;

    for (size_t i = 0; i < tapes.size(); ++i) {
        auto val = tapes[i]->read();
        if (val.has_value()) {
            heap.push({*val, i});
        }
    }

    output.rewind();
    bool first = true;

    while (!heap.empty()) {
        auto [value, tape_idx] = heap.top();
        heap.pop();

        if (!first) output.moveForward();
        output.write(value);
        first = false;

        if (tapes[tape_idx]->moveForward()) {
            auto next = tapes[tape_idx]->read();
            if (next.has_value()) {
                heap.push({*next, tape_idx});
            }
        }
    }
}