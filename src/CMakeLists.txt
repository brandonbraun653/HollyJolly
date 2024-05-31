add_library(pio_ws2812 INTERFACE)
pico_generate_pio_header(pio_ws2812 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

add_executable(HollyJolly
        main.cpp
        )

# pull in common dependencies
target_link_libraries(HollyJolly
        pico_multicore
        pico_stdlib
        hardware_pio
        hardware_dma
        pio_ws2812
)

# create map/bin/hex file etc.
pico_add_extra_outputs(HollyJolly)