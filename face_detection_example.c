// Copyright (c) 2022 Useful Sensors Inc.
// SPDX-License-Identifier: Apache-2.0
//
// Example code to communicate with a Useful Sensors' Person Sensor.
// See the full developer guide at https://usfl.ink/ps_dev for more information.
//   
// Demonstrates how to set up and receive information from a person sensor. 
// Canonical home for this code is at 
// https://github.com/usefulsensors/useful_pico_person_sensor

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/i2c.h"

#include "person_sensor.h"

// Speed of the I2C bus. The sensor supports many rates but 400KHz works.
const int32_t I2C_BAUD_RATE = (400 * 1000);

// How long to wait between reading the sensor. The sensor can be read as
// frequently as you like, but the results only change at about 5FPS, so
// waiting for 200ms is reasonable.
const int32_t SAMPLE_DELAY_MS = 200;

int main() {
    stdio_init_all();

    printf("Setting up i2c\n");

    // Use I2C0 on the default SDA and SCL pins (6, 7 on a Pico).
    i2c_init(i2c_default, I2C_BAUD_RATE);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool.
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, 
      PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    printf("Setup done for i2c\n");

    person_sensor_results_t results = {};
    while (1) {
        // Perform a read action on the I2C address of the sensor to get the
        // current face information detected.
        if (!person_sensor_read(&results)) {
            printf("No person sensor results found on the i2c bus\n");
            sleep_ms(SAMPLE_DELAY_MS);
            continue;
        }

        printf("********\n");
        printf("%d faces found\n", results.num_faces);
        for (int i = 0; i < results.num_faces; ++i) {
            const person_sensor_face_t* face = &results.faces[i];
            printf("Face #%d: %d confidence, (%d, %d), (%d, %d), %s\n",
              i, face->box_confidence, face->box_left, face->box_top,
              face->box_right, face->box_bottom, 
              face->is_facing ? "facing" : "not facing");
        }

        sleep_ms(SAMPLE_DELAY_MS);
    }
    return 0;
}
