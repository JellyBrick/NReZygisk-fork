#include <unistd.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

#include "utils.h"

std::string last_mountinfo;

static bool read_file(const char* path, std::string& data) {
    data = "";
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd == -1) return false;

    char buffer[4096];
    ssize_t n;

    while ((n = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)))) > 0) {
        data.append(buffer, static_cast<std::size_t>(n));
    }

    close(fd);
    return n == 0;
}

static bool check_file(const char *path, std::string &expected) {
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd == -1) return false;

    char buffer[4096];
    ssize_t n;
    size_t offset = 0;

    while ((n = TEMP_FAILURE_RETRY(read(fd, buffer, sizeof(buffer)))) > 0) {
        if (offset + (size_t) n > expected.size() ||
            memcmp(buffer, expected.data() + offset, (size_t) n) != 0) {
            close(fd);
            return false;
        }
        offset += (size_t) n;
    }

    close(fd);
    return n == 0 && offset == expected.size();
}

void sync_mns(int inner_ns) {
    if (access(TMP_PATH "/private_mounts", F_OK) != 0) return;

    if (!last_mountinfo.empty() && check_file("/proc/1/mountinfo", last_mountinfo)) {
        return;
    }
    read_file("/proc/1/mountinfo", last_mountinfo);

    struct mountinfos outer_mi;
    if (!parse_mountinfo("1", &outer_mi)) return;

    int outer_ns = open("/proc/1/ns/mnt", O_RDONLY | O_CLOEXEC);
    if (outer_ns == -1) return;

    if (setns(inner_ns, CLONE_NEWNS) == -1) {
        close(outer_ns);
        return;
    }

    struct mountinfos inner_mi;
    if (!parse_mountinfo("self", &inner_mi)) {
        close(outer_ns);
        return;
    }

    if (setns(outer_ns, CLONE_NEWNS) == -1) {
        close(outer_ns);
        return;
    }

    std::unordered_set<std::string> inner_mp;
    std::unordered_set<std::string> outer_mp;

    for (size_t i = 0; i < inner_mi.length; ++i) {
        inner_mp.insert(inner_mi.mounts[i].target);
    }

    for (size_t i = 0; i < outer_mi.length; ++i) {
        if (outer_mi.mounts[i].optional.shared) continue;
        std::string mountpoint = outer_mi.mounts[i].target;
        if (mountpoint.rfind("/storage", 0) == 0) continue;
        if (inner_mp.find(mountpoint) != inner_mp.end()) continue;
        if (outer_mp.find(mountpoint) != outer_mp.end()) continue;

        int fd = open(mountpoint.c_str(), O_RDONLY | O_CLOEXEC);
        struct stat st;
        if (fstat(fd, &st) != 0 || st.st_dev != outer_mi.mounts[i].device) {
            close(fd);
            continue;
        }

        const char *tmp;
        if (S_ISDIR(st.st_mode)) tmp = TMP_PATH "/tmp/" lp_select("copy32_d", "copy64_d");
        else tmp = TMP_PATH "/tmp/" lp_select("copy32_f", "copy64_f");

        if (mount(mountpoint.c_str(), tmp, nullptr, MS_BIND, nullptr) == -1) {
            close(fd);
            continue;
        }

        setns(inner_ns, CLONE_NEWNS);
        mount(tmp, mountpoint.c_str(), nullptr, MS_BIND, nullptr);
        setns(outer_ns, CLONE_NEWNS);

        umount2(tmp, MNT_DETACH);
        outer_mp.insert(mountpoint);
        close(fd);
    }
}
