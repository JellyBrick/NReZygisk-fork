#include "../external/lsplt/lsplt/src/main/jni/elf_util.hpp"
#include "../external/lsplt/lsplt/src/main/jni/elf_util.cc"

/* INFO: This file is an ugly hack */

extern "C" void* elfplt_init(uintptr_t base_addr) {
    auto *elf = new Elf(base_addr);
    if (!elf->Valid()) {
        delete elf;
        return nullptr;
    }
    return (void*) elf;
}

extern "C" void* elfplt_addr(void *v_elf, const char *name) {
    Elf *elf = (Elf*) v_elf;
    auto addrs = elf->FindPltAddr(name);
    if (!addrs.empty()) {
        return (void*) addrs[0];
    }
    return nullptr;
}