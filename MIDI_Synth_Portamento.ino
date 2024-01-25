#include <MIDI.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <mozzi_midi.h>
#include <ADSR.h>
#include <Portamento.h> // Include Portamento library

MIDI_CREATE_DEFAULT_INSTANCE();

#define CONTROL_RATE 128 // Hz, powers of 2 are most reliable

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
ADSR <CONTROL_RATE, AUDIO_RATE> envelope;
Portamento <CONTROL_RATE> aPortamento; // Corrected template argument

#define LED 13 // shows if MIDI is being received

void HandleNoteOn(byte channel, byte note, byte velocity) {
  aPortamento.start(note); // Directly pass the MIDI note number
  envelope.noteOn();
  digitalWrite(LED, HIGH);
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  envelope.noteOff();
  digitalWrite(LED, LOW);
}

void setup() {
  pinMode(LED, OUTPUT);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);

  envelope.setADLevels(255, 64);
  envelope.setTimes(50, 200, 10000, 200);

  aSin.setFreq(440); // default frequency
  aPortamento.setTime(300u); // Set portamento time
  startMozzi(CONTROL_RATE);
}

void updateControl(){
  MIDI.read();
  envelope.update();
  
  // Use mtof to convert MIDI note to frequency
  // aPortamento.next() returns a new MIDI note number in the portamento sequence
  aSin.setFreq(mtof((uint8_t)aPortamento.next()));
}

int updateAudio(){
  return (int) (envelope.next() * aSin.next()) >> 8;
}

void loop() {
  audioHook(); // required here
}
