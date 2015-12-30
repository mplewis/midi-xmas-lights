#define ENTROPY_PIN A5

// RGB Plasma
byte offset = 0;     // counter for radial color wave motion
int plasVector = 0;  // counter for orbiting plasma center
void plasma() {
  // startup tasks
  if (effectInit == false) {
    effectInit = true;
    effectDelay = 40;
  }

  // Calculate current center of plasma pattern (can be offscreen)
  int xOffset = cos8(plasVector / 256);
  int yOffset = sin8(plasVector / 256);

  // Draw one frame of the animation into the LED array
  static const int y = 0;
  for (int x = 0; x < NUM_LEDS; x++) {
    byte color = sin8(sqrt(sq(((float)x - 7.5) * 10 + xOffset - 127) +
                           sq(((float)y - 2) * 10 + yOffset - 127)) +
                      offset);
    leds[led(x)] = CHSV(color, 255, 255);
  }

  offset++;          // wraps at 255 for sin8
  plasVector += 1;  // using an int for slower orbit (wraps at 65536)
}

// Scanning pattern left/right, uses global hue cycle
byte riderPos = 0;
void rider() {
  // startup tasks
  if (effectInit == false) {
    effectInit = true;
    effectDelay = 30;
    riderPos = 0;
  }

  // Draw one frame of the animation into the LED array
  for (byte x = 0; x < NUM_LEDS; x++) {
    int brightness = (abs(x * (256 / NUM_LEDS) -
                      triwave8(riderPos) * 2 + 127)
                      * 3);
    if (brightness > 255) brightness = 255;
    brightness = 255 - brightness;
    CRGB riderColor = CHSV(cycleHue, 255, brightness);
    leds[x] = riderColor;
  }

  riderPos++;  // byte wraps to 0 at 255, triwave8 is also 0-255 periodic
}

// Pixels with random locations and random colors selected from a palette
// Use with the fadeAll function to allow old pixels to decay
void confetti() {
  // startup tasks
  if (effectInit == false) {
    effectInit = true;
    effectDelay = 1;
  }

  if (random16(15) != 0) return;
  random16_add_entropy(analogRead(ENTROPY_PIN));
  leds[random16(NUM_LEDS)] =
    ColorFromPalette(currentPalette, random16(255), 255);
}

// Draw slanting bars scrolling across the array, uses current hue
byte slantPos = 0;
void slantBars() {
  // startup tasks
  if (effectInit == false) {
    effectInit = true;
    effectDelay = 10;
  }

  for (byte x = 0; x < NUM_LEDS; x++) {
    leds[led(x)] = CHSV(cycleHue, 255, quadwave8(x * 4 + slantPos));
  }

  slantPos -= 1;
}

void confettiMidiOn(uint8_t intensity) {
  random16_add_entropy(analogRead(ENTROPY_PIN));
  uint8_t count = map(intensity, 0, 127, 1, 6);
  uint8_t brightness = dim8_raw(map(intensity, 0, 127, 32, 255));
  for (int i = 0; i < count; i++)
    leds[random16(NUM_LEDS)] =
      ColorFromPalette(currentPalette, random16(255), brightness);
}

unsigned int seq_pos = 0;
void sequentialMidiOn(uint8_t intensity) {
  uint8_t brightness = dim8_raw(map(intensity, 0, 127, 32, 255));
  leds[seq_pos] = ColorFromPalette(currentPalette, random16(255), brightness);
  seq_pos = (seq_pos + 1) % NUM_LEDS;
}

void oneToOneMidiOn(uint16_t note, uint8_t intensity) {
  uint16_t pos = note - 0x15; // lowest note on the 88-key MIDI keyboard
  uint8_t brightness = dim8_raw(map(intensity, 0, 127, 32, 255));
  leds[pos] = ColorFromPalette(currentPalette, random16(255), brightness);
}

void splashMidiOn(uint16_t note, uint8_t intensity) {
  // lowest note on the 88-key MIDI keyboard plus 4 on the outside
  uint16_t pos = note - 0x19;
  uint8_t asideCount = map(intensity, 0, 127, 0, 4);
  uint8_t brightness = dim8_raw(map(intensity, 0, 127, 32, 255));
  for (int i = pos - asideCount; i <= pos + asideCount; i++) {
    leds[i] = ColorFromPalette(currentPalette, random16(255), brightness);
  }
}
