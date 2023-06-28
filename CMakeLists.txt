cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ../../pico-sdk)
set(PICO_EXTRAS_PATH ../../pico-extras)
set(PICO_PLAYGROUND_PATH ../../pico-playground)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)
include(pico_playground_import.cmake)

project(picos C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(picos
        src/main.cpp
        src/font8.c
        src/font6.c
        src/font10.c
        )

target_compile_definitions(picos PRIVATE
        PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4
        PICO_SCANVIDEO_PLANE1_FIXED_FRAGMENT_DMA=true
        PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
        #PICO_SCANVIDEO_48MHZ
)

pico_add_extra_outputs(picos)

pico_enable_stdio_usb(picos 1)
pico_enable_stdio_uart(picos 0)


target_link_libraries(picos PRIVATE
        pico_multicore
        pico_stdlib
        pico_scanvideo_dpi
        render)

include_directories(picos ../pico-playground/scanvideo/render include)