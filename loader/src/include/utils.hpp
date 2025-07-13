#ifndef REZYGISK_UTILS_HPP
#define REZYGISK_UTILS_HPP

#include <cstddef>

struct MappedBuffer {
    char *data;
    size_t size;
    size_t capacity;

    MappedBuffer() : data(nullptr), size(0), capacity(0) {}

    MappedBuffer(MappedBuffer &&other) noexcept
            : data(other.data), size(other.size), capacity(other.capacity) {
        other.data = nullptr;
        other.size = 0;
        other.capacity = 0;
    }

    MappedBuffer &operator=(MappedBuffer &&other) noexcept {
        if (this != &other) {
            data = other.data;
            size = other.size;
            capacity = other.capacity;
            other.data = nullptr;
            other.size = 0;
            other.capacity = 0;
        }
        return *this;
    }

    bool grow(size_t target);

    void unmap();

    bool file_read(const char *path, size_t expected_len);

    bool operator==(MappedBuffer const &rhs) const;

    MappedBuffer(const MappedBuffer &) = delete;

    MappedBuffer &operator=(const MappedBuffer &) = delete;
};

#endif //REZYGISK_UTILS_HPP
