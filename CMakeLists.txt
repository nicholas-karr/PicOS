cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ../../pico-sdk)
set(PICO_EXTRAS_PATH ../../pico-extras)
set(PICO_PLAYGROUND_PATH ../../pico-playground)




#Wireless stuff

set(PICO_BOARD pico_w)
file(READ wifi_ssid.txt WIFI_SSID)
file(READ wifi_password.txt WIFI_PASSWORD)

set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)
set(BTSTACK_EXAMPLE_PATH ${BTSTACK_ROOT}/example)
set(BTSTACK_3RD_PARTY_PATH ${BTSTACK_ROOT}/3rd-party)
set(BT_EXAMPLE_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}")




# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)
#include(pico_playground_import.cmake)

project(picos C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_executable(picos
        src/text.S
        src/main.cpp
        src/font8.c
        src/font6.c
        src/font10.c  
        )

target_compile_definitions(picos PRIVATE
        PICO_SCANVIDEO_SCANLINE_BUFFER_COUNT=4
        PICO_SCANVIDEO_MAX_SCANLINE_BUFFER_WORDS=360
        
        PICO_SCANVIDEO_PLANE_COUNT=1

        PICO_SCANVIDEO_MAX_SCANLINE_BUFFER1_WORDS=360
        PICO_SCANVIDEO_PLANE1_FRAGMENT_DMA=true
        #PICO_SCANVIDEO_PLANE1_FIXED_FRAGMENT_DMA=true
        PICO_SCANVIDEO_PLANE1_VARIABLE_FRAGMENT_DMA=true

        PICO_SCANVIDEO_MAX_SCANLINE_BUFFER2_WORDS=60
        PICO_SCANVIDEO_PLANE2_FRAGMENT_DMA=true
        PICO_SCANVIDEO_PLANE2_FIXED_FRAGMENT_DMA=true
        #PICO_SCANVIDEO_PLANE2_VARIABLE_FRAGMENT_DMA=true
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
        pico_scanvideo_dpi

        pico_btstack_classic 
        pico_btstack_ble
        pico_btstack_cyw43
        pico_cyw43_arch_none
        )

include_directories(picos ../pico-playground/scanvideo/render include)