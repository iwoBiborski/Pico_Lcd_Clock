cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(Pico_ePaper_Code)
pico_sdk_init()

#添加编译子目录
add_subdirectory(lib/Config)
add_subdirectory(lib/Infrared)
add_subdirectory(lib/Icm20948)
add_subdirectory(lib/LCD)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
add_subdirectory(Pico_Clock)



#添加头文件目录
include_directories(Pico_Clock)
include_directories(./lib/Config)
include_directories(./lib/GUI)
include_directories(./lib/Icm20948)
include_directories(./lib/LCD)
include_directories(./lib/Infrared)

# 生成可执行文件
add_executable(main 
main.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(main)

target_link_libraries(main Pico_Clock LCD Infrared Icm20948 GUI Fonts Config pico_stdlib hardware_spi hardware_uart)