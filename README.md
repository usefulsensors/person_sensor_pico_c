# Using a Person Sensor on a Raspberry Pi Pico

Example code that shows how to interface the Raspberry Pi Pico to the person 
sensor board over I2C.

## Introduction

The Person Sensor is a small hardware module that's intended to make it easy to
find out when people are near a device, where they are, and who they are. It has
an image sensor and a microcontroller with pretrained ML models that use
computer vision to spot faces. 

There's a [detailed developer guide](https://usfl.ink/ps_dev)
available, but this project has sample code that shows you specifically how to 
get the sensor up and running with a Raspberry Pi Pico using C.

## Building

Make sure that you're able to build and run the standard Raspberry Pi Pico
examples, like `blink`. You can find general instructions on this in the 
[Pico Getting Started Guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf),
and [here's a Colab notebook that might help](https://usfl.ink/pico_blink_colab).
The main issues to watch out for are ensuring that `PICO_SDK_PATH` is set as an
environment variable, and points to the right location.

Once you're ready, run the following commands from within this repository's
folder to create the build files:

```bash
mkdir build
cd build
cmake ..
```

After that succeeds, you should be able to run the compilation stage:

```bash
make
```

There should now be a binary at `build/face_detection_example.utf2`. You can
install this on your Pico board, though it won't work until you wire up the
sensor to the right pins.

## Wiring information

Wiring up the device requires 4 jumpers, to connect VDD, GND, SDA and SCL. The 
example here uses I2C port 0, which is assigned to GPIO4 (SDA, pin 6) and GPIO5
(SCL, pin 7) in software. Power is supplied from 3V3(OUT) (pin 36), with ground
attached to GND (pin 38).

Follow the wiring scheme shown below:

![Wiring diagram for Person Sensor/Pico](pico_person_sensor_bb.png)

If you're using [Qwiic connectors](https://www.sparkfun.com/qwiic), the colors 
will be black for GND, red for 3.3V, blue for SDA, and yellow for SDC.

## Running

Once you have the sensor wired up, connect the Pico over USB while holding the
`bootsel` button to mount it as a storage device, copy the 
`face_detection_example.utf2` file over to it, and it should begin running. To 
see the logging output you'll need to set up `minicom` or a similar tool. Once
that is done, you should start to see information about the faces it spots, or
error messages.

## Troubleshooting

### Power

The first thing to check is that the sensor is receiving power through the
`VDD` and `GND` wires. The simplest way to test this is to hold the sensor
upright (so the I2C connector is at the top) and point it at your face. You
should see a green LED light up. If you don't see any response from the LED then
it's likely the sensor isn't receiving power, so check those wires are set up
correctly.

### Communication

If you see connection errors when running the face detection example, you may
have an issue with your wiring. To help track down what's going wrong, you can
copy over the `scan_i2c.utf2` file to the board, and this will display which
I2C devices are available in the logs. Here's an example from a board that's set
up correctly:

```
00 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .                       
10 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .                       
20 .  .  .  .  @  .  .  .  .  .  .  .  .  .  .  .                       
30 .  .  .  .  .  .  .  .  .  @  .  .  .  .  .  .                       
40 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  @                       
50 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .                       
60 .  .  @  .  .  .  .  .  @  .  .  .  .  .  .  .                       
70 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
```

The important entry is the first `@` shown on line starting with `60`. This
indicates that there's a response on the address `0x62`, which is the fixed
location of the person sensor. If the `@` isn't present at this point in the
grid then it means the sensor isn't responding to I2C messages as it should be.
The most likely cause is that there's a wiring problem, so if you hit this you
should double-check that the SDA and SCL wires are going to the right pins.