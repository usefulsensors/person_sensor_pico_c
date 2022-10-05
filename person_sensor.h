#ifndef INCLUDE_PERSON_SENSOR_H
#define INCLUDE_PERSON_SENSOR_H

// Definitions for the Useful Sensors Person Sensor module.
// Includes the standard I2C address of the sensor, constants for the
// configuration commands, and the data structures used to communicate results
// to the main system.
// See the full developer guide at https://usfl.ink/ps_dev for more information.

#include <stdint.h>

// The I2C address of the person sensor board.
const uint8_t PERSON_SENSOR_I2C_ADDRESS = 0x62;

// Configuration commands for the sensor. Write this as a byte to the I2C bus
// followed by a second byte as an argument value.
const uint8_t REG_MODE = 0x01;
const uint8_t REG_SINGLE_SHOT = 0x02;
const uint8_t REG_ENABLE_ID = 0x03;
const uint8_t REG_LABEL_ID = 0x04;
const uint8_t REG_ENABLE_SMOOTHING = 0x05;

// The person sensor will never output more than four faces.
#define MAX_FACES_COUNT (4)

// The following structures represent the data format returned from the person
// sensor over the I2C communication protocol. The C standard doesn't
// guarantee the byte-wise layout of a regular struct across different
// platforms, so we add the non-standard (but widely supported) __packed__
// attribute to ensure the layouts are the same as the wire representation.

// The results returned from the sensor have a short header providing
// information about the length of the data packet:
//   reserved: Currently unused bytes.
//   data_size: Length of the entire packet, excluding the header and checksum.
//     For version 1.0 of the sensor, this should be 40.
typedef struct __attribute__ ((__packed__)) {
    uint8_t reserved[2];  // Bytes 0 to 1.
    uint16_t data_size;   // Bytes 2 to 3.
} person_sensor_results_header_t;

// Each face found has a set of information associated with it:
//   box_confidence: How certain we are we have found a face, from 0 to 255.
//   box_left: X coordinate of the left side of the box, from 0 to 255.
//   box_top: Y coordinate of the top edge of the box, from 0 to 255.
//   box_width: Width of the box, where 255 is the full view port size.
//   box_height: Height of the box, where 255 is the full view port size.
//   id_confidence: How sure the sensor is about the recognition result.
//   id: Numerical ID assigned to this face.
//   is_looking_at: Whether the person is facing the camera, 0 or 1.
typedef struct __attribute__ ((__packed__)) {
    uint8_t box_confidence;   // Byte 1.
    uint8_t box_left;         // Byte 2.
    uint8_t box_top;          // Byte 3.
    uint8_t box_width;        // Byte 4.
    uint8_t box_height;       // Byte 5.
    uint8_t id_confidence;    // Byte 6.
    int8_t id;                // Byte 7
    uint8_t is_facing;        // Byte 8.
} person_sensor_face_t;

// This is the full structure of the packet returned over the wire from the
// sensor when we do an I2C read from the peripheral address.
// The checksum should be the CRC16 of bytes 0 to 38. You shouldn't need to
// verify this in practice, but we found it useful during our own debugging.
typedef struct __attribute__ ((__packed__)) {
    person_sensor_results_header_t header;        // Bytes 0 to 4.
    int8_t num_faces;                             // Byte 5.
    person_sensor_face_t faces[MAX_FACES_COUNT];  // Bytes 6 to 37.
    uint16_t checksum;                            // Bytes 38 to 39.
} person_sensor_results_t;

#endif  // INCLUDE_PERSON_SENSOR_H