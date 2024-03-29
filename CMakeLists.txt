cmake_minimum_required(VERSION 3.13)

# Change your executable name to something creative!
set(NAME flipclock) # <-- Name your project/executable here!

include(pimoroni_pico_import.cmake)
include(pico_sdk_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your source files
add_executable(${NAME}
    main.cpp # <-- Add source files here!
    input.cpp 
    output.cpp 
    ui.cpp 
    clock.cpp
    background.cpp 
    sunutils.cpp
)

add_subdirectory( MemI2C )
add_subdirectory( DS3231_HAL )

# Include required libraries
# This assumes `pimoroni-pico` is stored alongside your project
# include(libraries/pico_explorer/pico_explorer)
include(libraries/pico_display_2/pico_display_2)

# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    pico_stdlib hardware_spi hardware_pwm hardware_dma pico_display_2 hardware_rtc hardware_i2c# <-- List libraries here!
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)

