add_library(optimized-routines STATIC
        aarch64/__mtag_tag_region.S
        aarch64/__mtag_tag_zero_region.S
        aarch64/check-arch.S
        aarch64/memchr.S
        aarch64/memchr-mte.S
        aarch64/memchr-sve.S
        aarch64/memcmp.S
        aarch64/memcmp-sve.S
        aarch64/memcpy.S
        aarch64/memcpy-advsimd.S
        aarch64/memcpy-mops.S
        aarch64/memcpy-sve.S
        aarch64/memmove-mops.S
        aarch64/memrchr.S
        aarch64/memset.S
        aarch64/memset-mops.S
        aarch64/stpcpy.S
        aarch64/stpcpy-sve.S
        aarch64/strchr.S
        aarch64/strchr-sve.S
        aarch64/strchrnul.S
        aarch64/strchrnul-sve.S
        aarch64/strcmp.S
        aarch64/strcmp-sve.S
        aarch64/strcpy.S
        aarch64/strcpy-sve.S
        aarch64/strlen.S
        aarch64/strlen-sve.S
        aarch64/strncmp.S
        aarch64/strncmp-sve.S
        aarch64/strnlen.S
        aarch64/strnlen-sve.S
        aarch64/strrchr.S
        aarch64/strrchr-sve.S
)
