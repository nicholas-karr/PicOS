# PicOS

This is an implementation of a windowed desktop environment for the Raspberry Pi Pico. It supports multiple windows of text, occlusion, a background, and user input.

<img src="https://karrmedia.com/assets/pico/game.jpg" style="height:50vh">

## Overview

This system renders a sequence of windows (rectangular text buffers) to the screen, one scanline at a time. On every line, C++ code determines what windows are visible, and how they are occluded by other windows. Then, it sends a produced list to a highly optimized assembly routine that blits blocks of 4 characters from a bitmap font to a queue to be rendered via PIO. A resistor ladder translates the parallel digital output signals to an analog format, which is combined with a sync signal on a VGA connector. Currently, classic Snake is implemented using this text rendering and input hardware.

## Assembly

The hardware of this project consists of a prototyping board with a resistor ladder digital-analog converter and 3 buttons. The resistor ladder converts digital output on the red, green, and blue channels into a compliant VGA signal, which is connected to a VGA port on an external board. Optionally, this VGA signal may be converted into HDMI with a second adapter. This design is based on the VGA DAC schematic from the official [Hardware design with RP2040](https://datasheets.raspberrypi.com/rp2040/hardware-design-with-rp2040.pdf) guide.

To put this together, you'll need:
- A Raspberry Pi Pico (W optional) with headers
- Magnet Wire
- Soldering Iron + Solder
- Standard-Valued Resistors
- 3 x Momentary Pushbutton Switch
- 24x18 Protoboard
- 2 x 20 Pin Female Headers
- 6-Pin Male Right Angle Header
- Male-Female Jumper Wires
- VGA Screw Terminal Block Adapter (Recommended)
- VGA to HDMI Adapter (Optional)

Assemble the protoboard according to the schematic below. If the resistor values are difficult to read, please consult the PDF above. The pin numbering have been kept the same. You do need to use the pin headers, as they provide space for the resistors and allow for the Pico to be removed.

The resistor on the back is optional. For the connections between distant pads, I recommend removing the insulation from the ends of a length of magnet wire (high gauge and enameled) and using it to bridge the pads.

<img src="https://karrmedia.com/assets/pico/layout.jpg" style="height:30vh">
<img src="https://karrmedia.com/assets/pico/front.jpg" style="height:30vh">
<img src="https://karrmedia.com/assets/pico/back.jpg" style="height:30vh">

## Installation

### Building Firmware

First, clone the repository.

```bash
git clone --recurse-submodules https://github.com/nicholas-karr/PicOS.git
```

#### With IDE

Open the new directory in Visual Studio Code. Install the Raspberry Pi Pico VS Code extension. More detailed instructions are available at the [Getting Started Guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) on page 5. In the extension's sidebar, run Configure CMake and then Compile Project. 

#### Without IDE

Ensure you have the library dependencies to build pico-sdk projects installed.

```bash
mkdir build
cd build
cmake .. -G Ninja
ninja
```

### Flashing

Plug in your Pico with the BOOTSEL button held. Copy `build/picos.uf2` to the USB storage it presents. Once it completes, the Pico will automatically restart and run the program.

## Low-level Code Explanation

The pico_scan_video library provided by Raspberry Pi is capable of rendering VGA scanlines using a specialized assembly language that runs on a peripheral block, the PIO. The PIO is suitable for a wide range of medium-difficulty protocols that are too complex to perform purely through GPIO but don't require dedicated silicon. Other popular protocols include USB, Ethernet, and analog TV broadcast. 

This code reads tokens from a buffer at a specific clock, where each 32-bit token contains the color for 1-2 pixels and 0-1 control tokens. These tokens may be placed in a buffer for the PIO through either direct or indirect DMA. With direct DMA, a second peripheral [(Turing complete!)](https://people.ece.cornell.edu/land/courses/ece4760/RP2040/C_SDK_DMA_machine/DMA_machine_rp2040.html) copies the stream of tokens generated for each scanline to a blittable buffer regularly. Multiple token streams may exist in parallel for dual-core rendering, but the PIO needs to be fed at specific intervals. This system is used for the background, as the solid color may be represented in very few total tokens (32). For the text, copying every line of every letter directly would take too much time. As such, it is rendered through indirect DMA: the token stream is filled with pointers to blocks of tokens, which are copied to the render buffer by DMA. Here, the fragment size is 4x32 bit words, which meshes well with the 8x16 bit size of a row of a letter. The pure pixel data letters are easy, while the control sequences are more difficult. They are declared in advance and modified on every line as needed. 

In the optimized assembly, blocks of 4x1 byte letters are copied at a time into registers to avoid excess memory fetches. Then, 4 stores are made after calculating the pointer index into the font for each character: one 32-bit pointer per character. In testing, fetching 4 bytes instead of 1 word incurred too much memory latency and prevented full lines from being rendered in time. There is no cache past the RAM on the RP2040, so minimizing memory access is essential. The 32-bit word size is as large a unit as can be stored.

To allow a background layer and a text layer to exist at the same time, I had to fork the pico-extras repo. The programmers never finished the code for DMA to be used with layers other than 1, so somewhat minor modifications were required.

HDMI/DVI implementations do exist for the RP2040, but I opted not to use them. This is because they restrict the effective resolution by requiring the use of a frame buffer. By using the significantly more difficult scanline-based rendering, I can avoid storing an entire full-color frame. This significantly improves text readability.

This entire project is linked with the `copy_to_ram` binary type since it was tested to improve frame times. The RP2040 does have a flash code cache, but it can still be improved on. This significantly restricts the code size that can be included in the binary. Future expansions could emulate the same effect by tracking every function that is called from the render core and individually marking them for placement in RAM. In this iteration, I opted to cut excess features. Surprisingly, USB serial support and soft floating point still fit!

### Interlacing

Even though 720p30 VGA is completely illegal and unsupported by every system I could find, it would save a lot of render time to only have to produce half as many scan lines. This can be implemented by simply skipping (i.e. no pixels are asserted) every other scan line, and alternating whether even or odd lines are skipped every frame. This worked in testing but did produce artifacts (an odd ethereal quality) since my digital monitor did not manually implement support for it. It may work better on a CRT.

<img src="https://karrmedia.com/assets/pico/interlaced.jpg" alt="Text boxes where one has a strong moiré and the other is vertically offset" style="height:50vh">

## Troubleshooting

- If nothing is rendered onscreen, use a signal analyzer to check that a valid VGA signal is being transmitted. Good luck.
- If you see alternating bright blue lines, which may or may not be moving every frame, you are not meeting the frame timings. You will need to speed up the rendering loop somehow.
- If you have a second Pico, the Picoprobe is a great way to debug the code.

## Potential Improvements

The portion of the code that converts the windows on a scanline into a list to be interpreted by the assembly routine could probably be replaced by a series of calls to said assembly, thus avoiding the memory pressure of creating and referencing the list. Also, the modules that were removed to cut code size (keyboard and mouse support) could be added back in with a custom linker map that places them in flash.

Rendering could also be moved to both cores if a good way to do the game calculations in the background was found. FreeRTOS on one core could be a viable option here.

This project was originally intended to be a word processor with a desktop, which was later changed because it had too many moving parts (would not work great with a three button interface demo). It could be brought back. Also, the icon layer needs to be rendered to actually see the mouse. Refer to git history if you're brave.

With some effort, the fourth button (BOOTSEL) could be used. It would require disabling the flash during reading, which should have nearly no effect as all functions are currently in RAM. I didn't do it because physically reaching said button is difficult.