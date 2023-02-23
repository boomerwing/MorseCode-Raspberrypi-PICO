# RP2040-FreeRTOS try 1.0.0

This repo contains my base project for [FreeRTOS](https://freertos.org/) on the Raspberry Pi RP2040 microcontroller

## Project Structure

```
/FreeRTOS-PICO
|___/App-CWKeyer            // Source code for App-CWKeyer

|___/App-CWTeach            // Source code for App-CWKeyer

|___/App-Keyer              // Source code for App-Keyer

|___/App-Shuffle            // Source code for App-Keyer

|___/App-SW-i2c             // Source code for App-SW-i2c

|___/App-TxtCtl             // Source code for App-TxtCtl

|___/Common                 // Source code common to applications
|
|___CMakeLists.txt          // Top-level project CMake config file
|___pico_sdk_import.cmake   // Raspberry Pi Pico SDK CMake import script
|
|___README.md
|___LICENSE.md
```

## Prerequisites

To use the code in this repo, your system must be set up for RP2040 C/C++ and FreeRTOS development. See [this blog post of "smittytone"](https://blog.smittytone.net/2021/02/02/program-raspberry-pi-pico-c-mac/) for setup details.

## Usage

1. Clone the repo: `git clone https://github.com/smittytone/RP2040-FreeRTOS`.
1. Enter the repo: `cd FreeRTOS-PICO`.
1. Install the submodules: `git submodule update --init --recursive`.
1. Edit `CMakeLists.txt` and `/<Application>/CMakeLists.txt` to rename the project.
1. Optionally, manually configure the build process: `cmake -S . -B build/`.
1. Optionally, manually build the app: `cmake --build build`.
1. Connect your device so it’s ready for file transfer.
1. Install the app (I use the Drag and Drop process described in the pico-sdk documentation)

## The Apps

This repo includes a number of deployable apps.

### App One: App-Keyer

This C app is a Morse Code CW Keyer used by Radio Amateurs to automatically transmit a repeated message in Morse Code. A number of messages can be stored and selected by switches.  The code shows three switches to select up to eight messages. A Seven Segment LED module shows the message number and two LED dots blink in response to the morse code being sent.

### App Two: App-Shuffle

This C app is a programming exercise to create a shuffle function. It can be set to shuffle any length character string (as shown it shuffles a complete Alphabet). It is controlled by two switches which are debounced in separate RTOS tasks. The output is displayed in the Minicom terminal App which is controlled by VT100 codes. The display shows how to use the switches to command the function.

### App Three: App-TxtCtl

This FreeRTOS C app is a programming exercise to create a Control function responding to text input from a PS2 Keyboard. An RTOS task reads the Keyboard with the RP2040 pio function in Common/PS2.c It commands LEDs in a seven segment display to blink.

### App Four: App-CWKeyer

This FreeRTOS C app is a Morse Code CW Keyer used by Radio Amateurs to automatically transmit a repeated message in Morse Code. A number of messages can be stored and selected by switches.  The code shows three switches to select up to eight messages. A Seven Segment LED module shows the message number. An LED blinks in response to the morse code being sent. This version of App-Keyer adds PS2 Keyboard input of a Message String in phrase No. 7. Enter the string while strings 0-6 are being output. The PS2 Keyboard is being managed by a PIO input.

### App Five: App-CWTeach

This FreeRTOS C app is a Morse Code CW Keyer used by Radio Amateurs to automatically transmit a repeated message in Morse Code. A number of messages can be stored and selected by switches.  The code shows three switches to select up to eight messages. A Seven Segment LED module shows the message number. An LED blinks in response to the morse code being sent. This version of App-Keyer adds a Message String random generated alphabet in phrase No. 7 (The eighth phrase). The first random string is initialized with two presses of the switch on Port 4 of the PIO expander used. Press the switch again to change the random string. Select the next random string while strings 0-6 are being output. The PS2 Keyboard is not implemented for this app.

### Common: Seven_seg.c

This code configures the Seven Segment LED module and displays the number (HEX or Decimal) sent to it.  My LED Module has two dots.  If your module has no dots or just one, you can use separate LEDs.

### Common: ps2.c
This code initializes, then manages the PS2 Keyboard input, decoding the keyboard scan codes and providing an ASCII character output.  This code can be called by an RTOS task.

### Common: pcf8575.c
This code initializes, then manages the pcf8575 port extender, driving the seven segment LED and providing input ports.


## Credits

This work has as its foundation the code provided by the smittytone/RP2040-FreeRTOS project.


## Copyright and Licences

Application source © 2022, Calvin McCarthy and licensed under the terms of the [MIT Licence](./LICENSE.md).

Application source © 2022, Tony Smith and licensed under the terms of the [MIT Licence](./LICENSE.md).

[FreeRTOS](https://freertos.org/) © 2021, Amazon Web Services, Inc. It is also licensed under the terms of the [MIT Licence](./LICENSE.md).

The [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) is © 2020, Raspberry Pi (Trading) Ltd. It is licensed under the terms of the [BSD 3-Clause "New" or "Revised" Licence](https://github.com/raspberrypi/pico-sdk/blob/master/LICENSE.TXT).
