// Copyright (c) 2022 Useful Sensors Inc.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/i2c.h"

// Example code to communicate with a Useful Sensors' Person Sensor
//   
// Demonstrates how to set up and receive information from a person sensor. 
// Canonical home for this code is at 
// https://github.com/usefulsensors/useful_pico_person_sensor

// GPIO PICO_DEFAULT_I2C_SDA_PIN (On Pico this is GP4 (physical pin 6)) -> SDA of person sensor
// GPIO PICO_DEFAULT_I2C_SCK_PIN (On Pico this is GP5 (physcial pin 7)) -> SCL of person sensor
// Vsys (physical pin 39) -> VDD on person sensor
// GND (physical pin 38)  -> GND on person sensor

// The I2C address of the person sensor board.
const uint8_t PERIPHERAL_ADDRESS = 0x62;

// Speed of the I2C bus. The sensor supports many rates but 400KHz works.
const int32_t I2C_BAUD_RATE = (400 * 1000);

// How long to wait between reading the sensor. The sensor can be read as
// frequently as you like, but the results only change at about 5FPS, so
// waiting for 200ms is reasonable.
const int32_t SAMPLE_DELAY_MS = 200;

// Configuration commands for the sensor. Write this as a byte to the I2C bus
// followed by a second byte as an argument value.
const uint8_t REG_MODE = 0x01;
const uint8_t REG_SINGLE_SHOT = 0x02;
const uint8_t REG_ENABLE_ID = 0x03;
const uint8_t REG_LABEL_ID = 0x04;
const uint8_t REG_ENABLE_SMOOTHING = 0x05;

// This is the structure of the packet returned over the wire from the sensor
// when we do an I2C read from the peripheral address. The C standard doesn't
// guarantee the byte-wise layout of a regular struct across different
// platforms, so we add the non-standard (but widely supported) __packed__
// attribute to ensure the layout is the same as the wire representation.
typedef struct __attribute__ ((__packed__)) {
  float confidence;     // Bytes 0 to 3
  float id_confidence;  // Bytes 4 to 7
  uint8_t x1;           // Byte 8
  uint8_t y1;           // Byte 9
  uint8_t x2;           // Byte 10
  uint8_t y2;           // Byte 11
  int8_t identity;      // Byte 12
} inference_results_t;

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

// Call this function from the main loop below to see what peripherals are
// available on the I2C bus, if you're having problems communicating with the
// person sensor.
void scan_i2c_bus() {
  for (int addr = 0; addr < (1 << 7); ++addr) {
    if (addr % 16 == 0) {
      printf("%02x ", addr);
    }

    // Perform a 1-byte dummy read from the probe address. If a slave
    // acknowledges this address, the function returns the number of bytes
    // transferred. If the address byte is ignored, the function returns
    // -1.

    // Skip over any reserved addresses.
    int ret;
    uint8_t rxdata;
    if (reserved_addr(addr)) {
      ret = PICO_ERROR_GENERIC;
    } else {
      ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);
    }
    printf(ret < 0 ? "." : "@");
    printf(addr % 16 == 15 ? "\n" : "  ");
  }
  printf("Done.\n");
}

int main() {
    stdio_init_all();

    printf("Setting up i2c\n");

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, I2C_BAUD_RATE);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool.
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    printf("Setup done for i2c\n");

    inference_results_t results;
    while (1) {
        // Perform a read action on the I2C address of the sensor to get the
        // current face information detected.
        int num_bytes_read = i2c_read_blocking(
            i2c_default,
            PERIPHERAL_ADDRESS, 
            (uint8_t*)(&results), 
            sizeof(inference_results_t), 
            false);
        // If there's no result from the sensor, don't print anything after the
        // first time.
        if (num_bytes_read != sizeof(inference_results_t)) {
            printf("No person sensor results found on the i2c bus\n");
            sleep_ms(SAMPLE_DELAY_MS);
            continue;
        }

        printf("Face confidence %f\% bbox [%d %d %d %d], id conf %f\% id %d\n", 
            results.confidence, 
            results.x1, results.y1, results.x2, results.y2, 
            results.id_confidence, results.identity);

        sleep_ms(SAMPLE_DELAY_MS);
    }
    return 0;
}
