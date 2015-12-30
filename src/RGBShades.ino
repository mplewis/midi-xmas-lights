#include <FastLED.h>
#include "XYmap.h"
#include "utils.h"
#include "effects.h"
#include "buttons.h"
#include "usb_midi.h"

#define COLOR_ORDER RGB
#define CHIPSET WS2811
#define BRIGHTNESS_DIAL A0
const byte LED_PIN = 5;

const byte PIN_MIDI_IN = 0;
const byte PIN_MIDI_OUT = 1;
const byte PIN_LED = 6;

const unsigned int cycleTime = 60000;
unsigned long midiTimeout = 30000;
const byte hueTime = 30; 

// fade-out rate. higher = longer, slower fades
const byte normalFadeRate = 2;
const byte pedalUpRate = 4;
const byte pedalDownRate = 10;
bool pedalDown = false;

byte fadeCount = 0;
unsigned long lastMidi = -midiTimeout;
bool midiEn;

bool midiEnabled() {
  return currentMillis - lastMidi < midiTimeout;
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  digitalWrite(PIN_LED, LOW);
  if (!midiEn) {
    fillAll(CRGB::Black);
  }
  lastMidi = currentMillis;
  splashMidiOn(pitch, velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  digitalWrite(PIN_LED, HIGH);
}

void handlePedalDown() {
  pedalDown = true;
}

void handlePedalUp() {
  pedalDown = false;
}

void setup() {
  Serial.begin(115200);

  // write FastLED configuration data
  // .setCorrection(TypicalSMD5050);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  // set global brightness value
  FastLED.setBrightness(127);

  // configure input buttons
  pinMode(MODEBUTTON, INPUT_PULLUP);
  pinMode(BRIGHTNESSBUTTON, INPUT_PULLUP);
  pinMode(PALETTEBUTTON, INPUT_PULLUP);
  pinMode(BRIGHTNESS_DIAL, INPUT);
  pinMode(ENTROPY_PIN, INPUT);
  pinMode(PIN_LED, OUTPUT);

  // start USB MIDI
  midi_setup();

  // pick a random palette at startup
  selectRandomPalette();
}

// list of functions that will be displayed
#define NUM_EFFECTS 4
functionList effectList[] = {plasma, confetti, rider, slantBars};

// Runs over and over until power off or reset
void loop() {
  currentMillis = millis();  // save the current timer value
  handle_usb_midi();         // read notes from MIDI

  // run a fade effect too if the confetti effect or MIDI is running
  byte fadeRate;
  if (midiEn) {
    if (pedalDown) {
      fadeRate = pedalDownRate;
    } else {
      fadeRate = pedalUpRate;
    }
  } else {
    fadeRate = normalFadeRate;
  }
  if (effectList[currentEffect] == confetti || midiEn) fadeCount++;
  if (fadeCount >= fadeRate) {
    fadeAll(1);
    fadeCount = 0;
  }

  updateButtons();           // read, debounce, and process the buttons

  // Check the mode button (for switching between effects)
  switch (buttonStatus(0)) {
    case BTNRELEASED:  // button was pressed and released quickly
      cycleMillis = currentMillis;
      if (++currentEffect >= NUM_EFFECTS)
        currentEffect = 0;  // loop to start of effect list
      effectInit =
          false;  // trigger effect initialization when new effect is selected
      break;

    case BTNLONGPRESS:         // button was held down for a while
      autoCycle = !autoCycle;  // toggle auto cycle mode
      confirmBlink();  // one blue blink: auto mode. two red blinks: manual
                       // mode.
      break;
  }

  // Check the palette switch button
  switch (buttonStatus(2)) {
    case BTNRELEASED:           // button was pressed and released quickly
      selectRandomPalette();
      fillAll(CRGB::Black);
      break;
  }

  FastLED.show();  // send the contents of the led memory to the LEDs

  // check the brightness dial
  if (currentMillis % 10 == 0) {
    int dial_raw = analogRead(BRIGHTNESS_DIAL);
    byte dial = map(dial_raw, 0, 1023, 0, 255);
    byte dimmed = dim8_raw(dial);
    FastLED.setBrightness(dimmed);
  }

  // skip fx if midi is being played right now
  midiEn = midiEnabled();
  if (midiEn) return;

  // switch to a new effect every cycleTime milliseconds
  if (currentMillis - cycleMillis > cycleTime &&
      autoCycle == true) {
    cycleMillis = currentMillis;
    // loop to start of effect list
    if (++currentEffect >= NUM_EFFECTS) currentEffect = 0;
    // trigger effect initialization when new effect is selected
    effectInit = false;
  }

  // increment the global hue value every hueTime milliseconds
  if (currentMillis - hueMillis > hueTime) {
    hueMillis = currentMillis;
    hueCycle(1);  // increment the global hue value
  }

  // Run the currently selected effect every effectDelay milliseconds
  if (currentMillis - effectMillis > effectDelay) {
    effectMillis = currentMillis;
    effectList[currentEffect]();  // run the selected effect function
    random16_add_entropy(1);  // make the random values a bit more random-ish
  }
}
