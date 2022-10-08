// Copyright (c) 2022 Useful Sensors Inc.
// SPDX-License-Identifier: Apache-2.0
//
// Example code to communicate with a Useful Sensors' Person Sensor.
// See the full developer guide at https://usfl.ink/ps_dev for more information.
//   
// Demonstrates how to use the Person Sensor for face recognition. 
// Canonical home for this code is at 
// https://github.com/usefulsensors/useful_pico_person_sensor

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "hardware/i2c.h"

#include "person_sensor.h"

// Speed of the I2C bus. The sensor supports many rates but 400KHz works.
#define I2C_BAUD_RATE (400 * 1000)

// How long to wait between reading the sensor. The sensor can be read as
// frequently as you like, but the results only change at about 5FPS, so
// waiting for 200ms is reasonable.
#define SAMPLE_DELAY_MS (200)

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

    // We look for a single unrecognized face visible for at least five frames
    // to add new recognition IDs. The calibration process takes about four
    // seconds.
    const int unrecognized_threshold = 5;
    int unrecognized_frame_count = 0;
    int next_unused_id = 1;
    const int calibration_threshold = 20;
    int calibration_frame_count = 0;

    while (1) {
        // Perform a read action on the I2C address of the sensor to get the
        // current face information detected.
        person_sensor_results_t results = {};
        if (!person_sensor_read(&results)) {
            printf("No person sensor results found on the i2c bus\n");
            sleep_ms(SAMPLE_DELAY_MS);
            continue;
        }

        printf("********\n");
        printf("%d faces found\n", results.num_faces);
        for (int i = 0; i < results.num_faces; ++i) {
            const person_sensor_face_t* face = &results.faces[i];
            if (face->id_confidence > 0) {
                printf("Recognized face %d as person %d with confidence %d\n", 
                    i, face->id, face->id_confidence);
            } else {
                printf("Unrecognized face %d\n", i);
            }
        }

        if (calibration_frame_count > 0) {
            // Let the sensor handle calibration for a few seconds if it has
            // been started.
            calibration_frame_count -= 1;
            if (calibration_frame_count == 0) {
                printf("Done calibrating\n");
            }
        } else if ((results.num_faces == 1) &&
            (results.faces[0].box_confidence >= 95) &&
            (results.faces[0].id_confidence <= 0) &&
            (results.faces[0].is_facing == 1) &&
            (next_unused_id < PERSON_SENSOR_MAX_IDS_COUNT))
        {
            // If we have an unrecognized face for a long enough time, start
            // the calibration process.
            unrecognized_frame_count += 1;
            if (unrecognized_frame_count >= unrecognized_threshold) {
                printf("Calibrating");
                person_sensor_write_reg(
                    PERSON_SENSOR_REG_CALIBRATE_ID, next_unused_id);
                calibration_frame_count = calibration_threshold;
                next_unused_id += 1;
            }
        } else {
            unrecognized_frame_count = 0;
        }

        sleep_ms(SAMPLE_DELAY_MS);
    }
    return 0;
}
