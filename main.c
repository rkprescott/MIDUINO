/*
 * MIDUINO.c
 *
 * Created: 3/14/2021 10:58:03 PM
 * Author : User
 */ 

#include <avr/io.h>
#include<MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
void setup() {
	Serial.begin(115200);
}
void loop() {
	MIDI.sendNoteOn(41, 127, 1);
	delay(500);
	MIDI.sendNoteOff(41, 127, 1);
	MIDI.sendNoteOn(38, 127, 1);
	delay(500);
	MIDI.sendNoteOff(38, 127, 1);
	MIDI.sendNoteOn(45, 127, 1);
	delay(500);
	MIDI.sendNoteOff(45, 127, 1);
}