
#ifndef INCBIN_H
#define INCBIN_H

#if defined(_MSC_VER)
#define INCBIN_ALIGN __declspec(align(16))
#else
#define INCBIN_ALIGN __attribute__((aligned(16)))
#endif

#define INCBIN_PREFIX incbin_
#define INCBIN_SECTION ".rodata"

#define INCBIN(name, file) \
    __asm__(".section " INCBIN_SECTION "\n" \
            ".global " #name "_start\n" \
            ".type " #name "_start, @object\n" \
            ".balign 16\n" \
            #name "_start:\n" \
            ".incbin \"" file "\"\n" \
            ".global " #name "_end\n" \
            ".type " #name "_end, @object\n" \
            ".balign 1\n" \
            #name "_end:\n" \
            ".byte 0\n" \
            ".text");

#define INCBIN_EXTERN(name) \
    extern const unsigned char name##_start[]; \
    extern const unsigned char name##_end[];

#define INCBIN_SIZE(name) ((size_t)((name##_end) - (name##_start)))

#endif // INCBIN_H
