# RP2040-FreeRTOS try 1.0.0

This repo contains my base project for [FreeRTOS](https://freertos.org/) on the Raspberry Pi RP2040 microcontroller

## Project Structure

```
/FreeRTOS-PICO
|___/App-CWKeyer            // Source code for App-CWKeyer

|___/App-CWTeach            // Source code for App-CWKeyer

|___/App-Keyer              // Source code for App-Keyer

|___/App-Shuffle            // Source code for App-Shuffle

|___/App-SW-i2c             // Source code for App-SW-i2c

|___/App-TxtCtl             // Source code for App-TxtCtl

|___/App-CWShuffle          // Source code for App-CWShuffle

|___/Common                 // Source code common to applications - pico
||
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

This C app is a programming exercise to create a shuffle function. It can be set to shuffle any length character string (as shown it shuffles a complete Alphabet). It is controlled by one switch which are debounced in a separate RTOS task (This task debounces eight switches attached to a PCF8575 GPIO extender and distributes the output through Queues. The Shuffle output is displayed in the Minicom terminal App which is controlled by VT100 codes. The display shows how to use the switches to command the function. As well there is a Blink task to blink the Pico LED and two other LEDs as an exercise with tasks.  The Blink task outputs the blink state into a Queue which does not empty so any task can look in the Queue to get a Blink command for its own use.

### App Three: App-TxtCtl

This FreeRTOS C app is a programming exercise to create a Control function responding to text input from a PS2 Keyboard. An RTOS task reads the Keyboard with the RP2040 pio function in Common/PS2.c It commands LEDs in a seven segment display to blink.  The Keyboard code is Based on [PS2Keyboard](https://github.com/PaulStoffregen/PS2Keyboard)


### App Four: App-CWKeyer

This FreeRTOS C app is a Morse Code CW Keyer used by Radio Amateurs to automatically transmit a repeated message in Morse Code. A number of messages can be stored and selected by switches.  The code shows three switches to select up to eight messages. A Seven Segment LED module shows the message number. An LED blinks in response to the morse code being sent. This version of App-Keyer adds PS2 Keyboard input of a Message String in phrase No. 7. Enter the string while strings 0-6 are being output. The PS2 Keyboard is being managed by a PIO input.

### App Five: App-CWTeach

This FreeRTOS C app is a Morse Code CW Keyer used by Radio Amateurs to automatically transmit a repeated message in Morse Code. A number of messages can be stored and selected by switches.  The code shows three switches to select up to eight messages. A Seven Segment LED module shows the active message number. An LED blinks in response to the morse code being sent. This version of App-Keyer adds a Message String random generated alphabet and numbers inserted into phrase No. 7 (The eighth phrase). To initialize the randomize process, two presses of the switch on Port 4 of the GPIO expander sets the initial random number. Then press the switch to put a new randomized string in phrase No. 7. The PS2 Keyboard is not implemented for this app.

### App Six: App-CWShuffle

This FreeRTOS C app is a Morse Code CW Keyer used by Radio Amateurs to automatically transmit a repeated message in Morse Code. A number of messages can be stored and selected by switches.  The code shows a select phrase task to select up to seven messages. A Seven Segment LED module shows the active message number. An LED blinks and an audio tone sounds in response to the morse code being sent. This version of App-Keyer adds a task to Shuffle the Message String selected before sending it to the CW generating task.  To initialize the randomize process, two presses of a switch sets the initial random number. A new randomized string is selected by the phrase select task. The PS2 Keyboard is not implemented for this app. The strings are Hard coded in the Shuffle task. A PIO function is used to generate a 600 Hz square wave as a tone oscillator. A discrete component NAND gate is given inputs from the Tone output GPIO and the CW code GPIO output.  The Code signal gates the square wave tone to output the audio Morse signal.  The blink.pio code is taken directly from pico-examples/pio/pio-blink.

### App-SWi2c
This code makes a number of exercises with the i2c GPIO extender with switches attached. It debounces the switch and makes toggle and On/Off functions.

### Common: Seven_seg.c
This code configures the Seven Segment LED module and displays the number (HEX or Decimal) sent to it by a Raspberry Pi PICO .  My LED Module has two dots.  If your module has no dots or just one, you can use separate LEDs. Uses the pcf8575 i2c GPIO Extender

### Common: ps2.c
This code initializes, then manages the PS2 Keyboard input, decoding the keyboard scan codes and providing an ASCII character output.  This code can be called by an RTOS task. This code is Based on [PS2Keyboard](https://github.com/PaulStoffregen/PS2Keyboard)


### Common: pcf8575.c
This code initializes, then manages the pcf8575 port extender, driving the seven segment LED and providing input ports.


## Credits

This work has as its foundation the code provided by the smittytone/RP2040-FreeRTOS project.


## Copyright and Licences

Application source © 2022, Calvin McCarthy and licensed under the terms of the [MIT Licence](./LICENSE.md).

Application source © 2022, Tony Smith and licensed under the terms of the [MIT Licence](./LICENSE.md).

[FreeRTOS](https://freertos.org/) © 2021, Amazon Web Services, Inc. It is also licensed under the terms of the [MIT Licence](./LICENSE.md).

The [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) is © 2020, Raspberry Pi (Trading) Ltd. It is licensed under the terms of the [BSD 3-Clause "New" or "Revised" Licence](https://github.com/raspberrypi/pico-sdk/blob/master/LICENSE.TXT).
