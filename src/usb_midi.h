#include <usbh_midi.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USB Usb;
USBH_MIDI Midi(&Usb);

extern void handleNoteOn(byte channel, byte pitch, byte velocity);
extern void handleNoteOff(byte channel, byte pitch, byte velocity);
void midi_poll();

void midi_setup() {
  if (Usb.Init() == -1) {
    Serial.println("USB initialization failed");
    while (1);  // halt
  }

  Serial.println("USB started");
}

void handle_usb_midi() {
  Usb.Task();
  if (Usb.getUsbTaskState() == USB_STATE_RUNNING) {
    midi_poll();
  }
}

// Poll USB MIDI Controler and send to serial MIDI
void midi_poll() {
  uint8_t bufMidi[64];
  uint16_t rcvd;

  uint8_t note = 0;
  // Set ignore to true to ignore an event that isn't note down/up
  // Note down is 0x09, 0x90, key, velocity (> 0)
  // Note up is   0x09, 0x90, key, 0
  bool ignore = false;
  if (Midi.RecvData(&rcvd, bufMidi) == 0) {
    for (unsigned char pos = 0; pos < 64; pos++) {
      uint8_t b = bufMidi[pos];

      if (pos % 4 == 0) {
        ignore = false;
        if (b == 0x0F || b == 0x00) break;
        if (b != 0x09) ignore = true;
        Serial.print("Event 1: ");
        Serial.println(b, HEX);

      } else if (pos % 4 == 1) {
        if (b != 0x90) ignore = true;
        Serial.print("Event 2: ");
        Serial.println(b, HEX);

      } else if (pos % 4 == 2) {
        Serial.print("Note: ");
        note = b;
        Serial.println(b, HEX);

      } else if (pos % 4 == 3) {
        Serial.print("Velocity: ");
        Serial.println(b, HEX);
        if (ignore) continue;
        if (b == 0) {
          handleNoteOff(0, note, b);
        } else {
          handleNoteOn(0, note, b);
        }
      }
    }
  }
}

