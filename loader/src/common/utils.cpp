#include <algorithm>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "utils.hpp"

bool MappedBuffer::grow(size_t target) {
    if (target <= capacity) return true;

    if (capacity == 0) {
        void *map = mmap(nullptr, target, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        if (map == MAP_FAILED) return false;
        data = (char *) map;
        capacity = target;
        prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, data, capacity, "zygisk_buf", 0);
        return true;
    }

    size_t new_capacity = capacity * 2;
    while (new_capacity < target) {
        if (new_capacity > std::numeric_limits<size_t>::max() / 2) {
            unmap();
            return false;
        }
        new_capacity *= 2;
    }

    void *map = mremap(data, capacity, new_capacity, MREMAP_MAYMOVE);
    if (map == MAP_FAILED) {
        unmap();
        return false;
    }

    data = (char *) map;
    capacity = new_capacity;
    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, data, capacity, "zygisk_buf", 0);
    return true;
}

void MappedBuffer::unmap() {
    if (capacity > 0) {
        munmap(data, capacity);
        data = nullptr;
        size = 0;
        capacity = 0;
    }
}

bool MappedBuffer::file_read(const char *path, size_t expected_len) {
    size = 0;
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd == -1) return false;

    if (!grow(expected_len + 4096)) {
        close(fd);
        return false;
    }

    ssize_t n;
    while ((n = TEMP_FAILURE_RETRY(read(fd, data + size, capacity - size))) > 0) {
        size += (size_t) n;
        if (!grow(size + 4096)) {
            close(fd);
            return false;
        }
    }

    close(fd);
    return n == 0;
}

bool MappedBuffer::operator==(MappedBuffer const &rhs) const {
    if (size != rhs.size) return false;
    return memcmp(data, rhs.data, size) == 0;
}
