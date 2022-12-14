cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(useful_pico_person_sensor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(face_detection_example
        face_detection_example.c
        )
target_link_libraries(face_detection_example pico_stdlib hardware_i2c)
pico_add_extra_outputs(face_detection_example)
pico_enable_stdio_usb(face_detection_example 1)

add_executable(scan_i2c
        scan_i2c.c
        )
target_link_libraries(scan_i2c pico_stdlib hardware_i2c)
pico_add_extra_outputs(scan_i2c)
pico_enable_stdio_usb(scan_i2c 1)

add_executable(face_recognition_example
        face_recognition_example.c
        )
target_link_libraries(face_recognition_example pico_stdlib hardware_i2c)
pico_add_extra_outputs(face_recognition_example)
pico_enable_stdio_usb(face_recognition_example 1)