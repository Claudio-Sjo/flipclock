include(${CMAKE_CURRENT_LIST_DIR}/system_deps.cmake)

set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m7)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
# Clang ARCH triple
set(ARCH armv7em-unknown-none-eabi)

# specify the cross compiler
set(CMAKE_C_COMPILER clang CACHE STRING "")
set(CMAKE_C_COMPILER_TARGET ${ARCH})
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "")
set(CMAKE_CXX_COMPILER_TARGET ${ARCH})
set(CMAKE_ASM_COMPILER clang CACHE STRING "")
set(CMAKE_ASM_COMPILER_TARGET ${ARCH})
set(CMAKE_SYSROOT "${CMAKE_CURRENT_SOURCE_DIR}/picolibc/arm-none-eabi")

set(CMAKE_EXECUTABLE_SUFFIX_C .elf)

# Machine type
add_compile_options(-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb)
# Optimization and language related
add_compile_options(-fshort-enums -Wall -Wextra -Wno-unused-parameter )
# Link related options
add_compile_options(-ffunction-sections -fdata-sections -ffreestanding)

add_link_options(
    -fuse-ld=lld -nostdlib
    LINKER:-marmelf
    LINKER:--gc-sections,--sort-section=alignment
)
