#include <gtest/gtest.h>
#include "FileTape.hpp"
#include "TapeSorter.hpp"
#include <filesystem>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

namespace fs = std::filesystem;

static TapeConfig testConfig() {
    TapeConfig cfg;
    cfg.memory_limit = 1;
    cfg.read_delay = cfg.write_delay = cfg.shift_delay = cfg.rewind_delay = std::chrono::milliseconds(0);
    return cfg;
}

static std::string tmpPath(const std::string& name) {
    return "/tmp/sorter_test_" + std::to_string(::getpid()) + "_" + name;
}

class SorterTest : public ::testing::Test {
protected:
    std::string input_path  = tmpPath("input.bin");
    std::string output_path = tmpPath("output.bin");
    std::string tmp_dir     = tmpPath("tmp");

    void TearDown() override {
        fs::remove(input_path);
        fs::remove(output_path);
        fs::remove_all(tmp_dir);
    }

    void writeInput(const std::vector<int32_t>& data) {
        FileTape tape(input_path, data.size(), testConfig());
        for (size_t i = 0; i < data.size(); ++i) {
            tape.write(data[i]);
            if (i + 1 < data.size()) tape.moveForward();
        }
    }

    std::vector<int32_t> readOutput(size_t size) {
        std::vector<int32_t> result;
        FileTape tape(output_path, testConfig());
        for (size_t i = 0; i < size; ++i) {
            auto val = tape.read();
            if (!val.has_value()) break;
            result.push_back(*val);
            tape.moveForward();
        }
        return result;
    }
};

TEST_F(SorterTest, SortSmallArray) {
    std::vector<int32_t> input = {5, 3, 1, 4, 2};
    writeInput(input);

    FileTape  in(input_path,  testConfig());
    FileTape  out(output_path, input.size(), testConfig());
    TapeSorter sorter(tmp_dir, testConfig());
    sorter.sort(in, out);

    auto result = readOutput(input.size());
    std::sort(input.begin(), input.end());
    EXPECT_EQ(result, input);
}

TEST_F(SorterTest, SortAlreadySorted) {
    std::vector<int32_t> input = {1, 2, 3, 4, 5};
    writeInput(input);

    FileTape  in(input_path,  testConfig());
    FileTape  out(output_path, input.size(), testConfig());
    TapeSorter sorter(tmp_dir, testConfig());
    sorter.sort(in, out);

    auto result = readOutput(input.size());
    EXPECT_EQ(result, input);
}

TEST_F(SorterTest, SortReversed) {
    std::vector<int32_t> input = {5, 4, 3, 2, 1};
    writeInput(input);

    FileTape  in(input_path,  testConfig());
    FileTape  out(output_path, input.size(), testConfig());
    TapeSorter sorter(tmp_dir, testConfig());
    sorter.sort(in, out);

    auto result   = readOutput(input.size());
    auto expected = input;
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(result, expected);
}

TEST_F(SorterTest, SortWithDuplicates) {
    std::vector<int32_t> input = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    writeInput(input);

    FileTape  in(input_path,  testConfig());
    FileTape  out(output_path, input.size(), testConfig());
    TapeSorter sorter(tmp_dir, testConfig());
    sorter.sort(in, out);

    auto result   = readOutput(input.size());
    auto expected = input;
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(result, expected);
}

TEST_F(SorterTest, SortSingleElement) {
    std::vector<int32_t> input = {42};
    writeInput(input);

    FileTape  in(input_path,  testConfig());
    FileTape  out(output_path, input.size(), testConfig());
    TapeSorter sorter(tmp_dir, testConfig());
    sorter.sort(in, out);

    auto result = readOutput(input.size());
    EXPECT_EQ(result, input);
}

TEST_F(SorterTest, SortLargeRandom) {
    const size_t N = 10000;
    std::vector<int32_t> input(N);
    std::mt19937 rng(42);
    std::generate(input.begin(), input.end(), rng);
    writeInput(input);

    FileTape  in(input_path,  testConfig());
    FileTape  out(output_path, N, testConfig());
    TapeSorter sorter(tmp_dir, testConfig());
    sorter.sort(in, out);

    auto result   = readOutput(N);
    auto expected = input;
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(result, expected);
}

TEST_F(SorterTest, SortTenMillionElementsReversed) {
    const size_t N = 10'000'000;
    std::cout << "[Test] Creating input tape with " << N
              << " elements (descending)...\n";
    {
        FileTape tape(input_path, N, testConfig());
        for (size_t i = 0; i < N; ++i) {
            tape.write(static_cast<int32_t>(N - i));
            if (i + 1 < N) tape.moveForward();
        }
    }
    std::cout << "[Test] Input tape created\n";

    std::cout << "[Test] Sorting...\n";
    {
        FileTape   in(input_path,  testConfig());
        FileTape   out(output_path, N, testConfig());
        TapeSorter sorter(tmp_dir, testConfig());
        sorter.sort(in, out);
    }
    std::cout << "[Test] Sort complete, verifying...\n";

    FileTape result(output_path, testConfig());

    auto val = result.read();
    ASSERT_TRUE(val.has_value()) << "Output tape is empty";
    EXPECT_EQ(*val, 1) << "First element should be 1";

    int32_t prev     = *val;
    size_t  count    = 1;
    size_t  errors   = 0;

    while (result.moveForward()) {
        val = result.read();
        ASSERT_TRUE(val.has_value()) << "Unexpected end at position " << count;

        if (*val < prev) {
            ++errors;
            if (errors <= 5) {
                ADD_FAILURE() << "Out of order at position " << count
                              << ": " << prev << " > " << *val;
            }
        }
        prev = *val;
        ++count;
    }

    EXPECT_EQ(count, N)   << "Element count mismatch";
    EXPECT_EQ(errors, 0u) << "Total ordering violations: " << errors;

    EXPECT_EQ(prev, static_cast<int32_t>(N))
        << "Last element should be " << N;

    std::cout << "[Test] Verified " << count << " elements, "
              << errors << " errors\n";
}