/*
 *  ESE350 MIDI Keyboard project made by some pretty cool folks,
 *	Richard Prescott & Alex GE
 *
 *  Version all arrays working
 */
#define F_CPU 16000000UL
#define BAUD_RATE 31250
#define BAUD_PRESCALER ((F_CPU / (BAUD_RATE * 16UL)) - 1)

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

char String[25];

#define midi_start 250
#define midi_stop 252
#define midi_clk 248

#define chan_on 144
#define chan_off 128
#define Vel 50

#define set(reg, bit) reg |= (1<<bit)
#define clear(reg, bit) reg &= ~(1<<bit)
#define check(reg, bit) reg & (1<<bit)
#define column_port PORTB
#define row_pin PIND

volatile int col[2] = {PORTB2, PORTB3};
volatile int row[4] = {PIND4, PIND5, PIND6, PIND7};
volatile int note[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile int send[8] = {1, 1, 1, 1, 1, 1, 1, 1};
volatile int notes[8] = {60, 62, 64, 65, 67, 69, 71, 72};

void UART_init(int prescaler) {
	
	//Set baud rate
	UBRR0H = (unsigned char)(prescaler>>8);
	UBRR0L = (unsigned char)prescaler;
	
	//1 stop bit
	UCSR0B = (1 << TXEN0);
	//8 data bits
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
}

void UART_send(unsigned char data) {
	//Wait for empty transmit buffer
	while(!(UCSR0A & (1<<UDRE0))) {
	}
	//Fill buffer and send
	UDR0 = data;
}

void Initialise() {
	//Set to column to output
	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
	//Set Row 1,2,3,4 to input
	DDRD &= ~(1<<DDD4);
	DDRD &= ~(1<<DDD5);
	DDRD &= ~(1<<DDD6);
	DDRD &= ~(1<<DDD7);
}

void ReadMatrix() {
	int count = 0;
	int i, j;
	for (i = 0; i < 2; i++) {
		set(column_port, col[i]);
		_delay_us(2);
		for (j = 0; j < 4; j++) {
			if (check(row_pin, row[j])) {
				note[count] = 1;
				} else {
				note[count] = 0;
			}
			count = count + 1;
		}
		clear(column_port, col[i]);
	}
}

int main(void) {
	//Note to future Alex, Spell with s not z
	Initialise();
	UART_init(BAUD_PRESCALER);
	while (1) {
		ReadMatrix();
		int i;
		for (i = 0; i < 8; i++) {
			if (note[i]) {
				if (send[i]) {
					UART_send(chan_on);
					UART_send(notes[i]);
					UART_send(Vel);
					send[i] = 0;
				}
			} else {
				UART_send(chan_off);
				UART_send(notes[i]);
				UART_send(0);
				send[i] = 1;
			}
		}
	}
}