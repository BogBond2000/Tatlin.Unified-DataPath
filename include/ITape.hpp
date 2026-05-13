#pragma once

#include <cstdint>
#include <optional>

class ITape {
public:
    virtual ~ITape() = default;

    virtual std::optional<int32_t> read()         = 0;
    virtual void write(int32_t) = 0;
    virtual bool moveForward()  = 0;
    virtual bool moveBackward() = 0;
    virtual void rewind() = 0;
    virtual size_t position() const = 0;
    virtual size_t size() const = 0;
};