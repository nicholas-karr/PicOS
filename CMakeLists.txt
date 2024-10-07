# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ../deps/pico-sdk)
set(PICO_EXTRAS_PATH ../deps/pico-extras)
set(PICO_PLAYGROUND_PATH ../deps/pico-playground)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(picos C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_executable(picos
        src/text.S
        src/mem.S
        src/main.cpp
        src/font8.c
        )

target_compile_definitions(picos PRIVATE
        PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4
        #PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=600
        
        PICO_SCANVIDEO_PLANE_COUNT=2
        
        # Layer 1 Background: Direct rendered
        PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=60

        # Layer 2 Text: DMA rendered
        PICO_SCANVIDEO_MAX_SCANLINE_BUFFER2_WORDS=600
        PICO_SCANVIDEO_PLANE2_FIXED_FRAGMENT_DMA=true

        # Layer 3 Icons: DMA rendered?
        #PICO_SCANVIDEO_MAX_SCANLINE_BUFFER3_WORDS=400
)

pico_add_extra_outputs(picos)

pico_enable_stdio_usb(picos 1)
pico_enable_stdio_uart(picos 0)

target_link_options(picos PUBLIC
        "-fno-fat-lto-objects"
        "-flto")

target_link_libraries(picos PRIVATE
        pico_multicore
        pico_stdlib
        pico_scanvideo_dpi)

include_directories(picos ../pico-playground/scanvideo/render include)