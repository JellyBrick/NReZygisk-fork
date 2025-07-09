project(external)

OPTION(LSPLT_BUILD_SHARED OFF)
add_subdirectory(lsplt/lsplt/src/main/jni)

add_library(phmap INTERFACE)
target_include_directories(phmap INTERFACE parallel-hashmap)
target_compile_options(phmap INTERFACE -Wno-unused-value)

add_library(xz-embedded STATIC
        xz-embedded/linux/lib/xz/xz_crc32.c
        xz-embedded/linux/lib/xz/xz_crc64.c
        xz-embedded/linux/lib/xz/xz_dec_lzma2.c
        xz-embedded/linux/lib/xz/xz_dec_stream.c
)
target_include_directories(xz-embedded PUBLIC xz-embedded/linux/include/linux)
target_include_directories(xz-embedded PRIVATE xz-embedded/userspace)
target_compile_options(xz-embedded PUBLIC -DXZ_USE_CRC64)
