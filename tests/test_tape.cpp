#include <gtest/gtest.h>
#include "FileTape.hpp"
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

static TapeConfig noDelayConfig() {
    TapeConfig cfg;
    cfg.read_delay   = std::chrono::milliseconds(0);
    cfg.write_delay  = std::chrono::milliseconds(0);
    cfg.shift_delay  = std::chrono::milliseconds(0);
    cfg.rewind_delay = std::chrono::milliseconds(0);
    cfg.memory_limit = 1;
    return cfg;
}

class FileTapeTest : public ::testing::Test {
protected:
    std::string path = "/tmp/test_tape_" + std::to_string(::getpid()) + ".bin";

    void TearDown() override {
        fs::remove(path);
    }
};

TEST_F(FileTapeTest, CreateAndReadWrite) {
    auto cfg = noDelayConfig();
    FileTape tape(path, 3, cfg);

    tape.write(10);
    tape.moveForward();
    tape.write(20);
    tape.moveForward();
    tape.write(30);

    tape.rewind();

    EXPECT_EQ(*tape.read(), 10);
    tape.moveForward();
    EXPECT_EQ(*tape.read(), 20);
    tape.moveForward();
    EXPECT_EQ(*tape.read(), 30);
}

TEST_F(FileTapeTest, MoveForwardAtEnd) {
    auto cfg = noDelayConfig();
    FileTape tape(path, 2, cfg);

    tape.write(1);
    tape.moveForward();
    tape.write(2);

    EXPECT_FALSE(tape.moveForward());
}

TEST_F(FileTapeTest, MoveBackwardAtStart) {
    auto cfg = noDelayConfig();
    FileTape tape(path, 2, cfg);

    EXPECT_FALSE(tape.moveBackward());
}

TEST_F(FileTapeTest, RewindResetsPosition) {
    auto cfg = noDelayConfig();
    FileTape tape(path, 3, cfg);

    tape.moveForward();
    tape.moveForward();
    EXPECT_EQ(tape.position(), 2u);

    tape.rewind();
    EXPECT_EQ(tape.position(), 0u);
}

TEST_F(FileTapeTest, SizeIsCorrect) {
    auto cfg = noDelayConfig();
    FileTape tape(path, 5, cfg);
    EXPECT_EQ(tape.size(), 5u);
}

TEST_F(FileTapeTest, ReadReturnsNulloptAtEnd) {
    auto cfg = noDelayConfig();
    FileTape tape(path, 1, cfg);

    tape.write(42);
    tape.moveForward();

    EXPECT_FALSE(tape.read().has_value());
}

TEST_F(FileTapeTest, MoveForwardHasDelay) {
    auto cfg = noDelayConfig();
    cfg.shift_delay = std::chrono::milliseconds(1);
    FileTape tape(path, 2, cfg);
    auto start = std::chrono::steady_clock::now();
    tape.moveForward();
    auto end = std::chrono::steady_clock::now();
    EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(), 1);
}