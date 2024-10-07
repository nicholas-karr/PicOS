# PicOS

```bash
mkdir build
cd build
cmake .. -G Ninja
ninja
```

## Installation

Install the Raspberry Pi Pico VS Code extension. More detailed instructions are available at the [Getting Started Guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf). In the extension's sidebar, run Configure CMake and then Compile Project. Plug in your Pico with the BOOTSEL button held. Copy `build/picos.uf2` to the drive it presents. Once it completes, the Pico will automatically restart and run the program.