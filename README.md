# MIDI Xmas Lights

Light up your tree when you play music on your USB keyboard. Play pretty animations when you're not at the piano.

# Hardware

* Arduino UNO or cheap clone
* String of WS2812B bulbs

# Usage

First, [install PlatformIO](http://docs.platformio.org/en/stable/installation.html).

Then, in the project root:

```sh
# Install dependencies
#     885: USBHost
#     126: FastLED
#     59: USB-Host-Shield-20
platformio lib install 885 126 59

# Compile and upload
platformio run  # add a specific port with --upload-port /dev/ttyMYDEVICE
```

# License

MIT

# Thanks

This project is based on Macetech's [RGB Shades firmware](https://github.com/macetech/RGBShades). Please buy these [awesome glasses](https://www.kickstarter.com/projects/macetech/rgb-led-shades).
