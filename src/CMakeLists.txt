add_library(pio_ws2812 INTERFACE)
pico_generate_pio_header(pio_ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

add_executable(HollyJolly
        animations/full_sweep_color_block.cpp
        animations/idle.cpp
        animations/twinkle.cpp
        animator.cpp
        buttons.cpp
        main.cpp
        ws2812.cpp
        )

# pull in common dependencies
target_link_libraries(HollyJolly
        hardware_dma
        hardware_pio
        pico_debug
        pico_multicore
        pico_stdio_usb
        pico_stdlib
        pio_ws2812
        tinyusb_board
        tinyusb_device
)

target_include_directories(HollyJolly PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# create map/bin/hex file etc.
pico_add_extra_outputs(HollyJolly)