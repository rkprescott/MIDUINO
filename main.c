/*
 *  ESE350 MIDI Keyboard project made by some pretty cool folks,
 *	Richard Prescott & Alex GE
 *
 *  Version works with partial arrays
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

volatile int note1 = 0;
volatile int note2 = 0;
volatile int note3 = 0;
volatile int note4 = 0;
volatile int note5 = 0;
volatile int note6 = 0;
volatile int note7 = 0;
volatile int note8 = 0;

volatile int send1 = 1;
volatile int send2 = 1;
volatile int send3 = 1;
volatile int send4 = 1;
volatile int send5 = 1;
volatile int send6 = 1;
volatile int send7 = 1;
volatile int send8 = 1;

volatile int col[2] = {PORTB2, PORTB3};
volatile int row[4] = {PIND4, PIND5, PIND6, PIND7};
volatile int note[8] = {0, 0, 0, 0, 0, 0, 0, 0};
volatile int send[8] = {1, 1, 1, 1, 1, 1, 1, 1};

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
	////column 1 on
	//PORTB |= (1<<PORTB2);
	//_delay_us(2);
	////row3
	//if (PIND & (1<<PIND4)) {
		//note3 = 1;
	//} else {
		//note3 = 0;
	//}
	////row4
	//if (PIND & (1<<PIND5)) {
		//note4 = 1;
	//} else {
		//note4 = 0;
	//}
	////row1
	//if (PIND & (1<<PIND6)) {
		//note1 = 1;
	//} else {
		//note1 = 0;
	//}
	////row2
	//if (PIND & (1<<PIND7)) {
		//note2 = 1;
		//} else {
		//note2 = 0;
	//}
	////column 1 off
	//PORTB &= ~(1<<PORTB2);
	//
	////column 2 on
	//PORTB |= (1<<PORTB3);
	//_delay_us(2);
	////row3
	//if (PIND & (1<<PIND4)) {
		//note7 = 1;
		//} else {
		//note7 = 0;
	//}
	////row4
	//if (PIND & (1<<PIND5)) {
		//note8 = 1;
		//} else {
		//note8 = 0;
	//}
	////row1
	//if (PIND & (1<<PIND6)) {
		//note5 = 1;
		//} else {
		//note5 = 0;
	//}
	////row2
	//if (PIND & (1<<PIND7)) {
		//note6 = 1;
		//} else {
		//note6 = 0;
	//}
	////column 2 off
	//PORTB &= ~(1<<PORTB3);
}

int main(void) {
	//Note to future Alex, Spell with s not z
	Initialise();
	UART_init(BAUD_PRESCALER);
	while (1) {
		ReadMatrix();
		
		if (note[0]) {
			if (send1) {
				UART_send(chan_on);
				UART_send(60);
				UART_send(Vel);
				send1 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(60);
			UART_send(0);
			send1 = 1;
		}
		if (note[1]) {
			if (send2) {
			UART_send(chan_on);
			UART_send(62);
			UART_send(Vel);
			send2 = 0;
			}	
		} else {
			UART_send(chan_off);
			UART_send(62);
			UART_send(0);
			send2 = 1;
		}
		if (note[2]) {
			if (send3) {
				UART_send(chan_on);
				UART_send(64);
				UART_send(Vel);
				send3 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(64);
			UART_send(0);
			send3 = 1;
		}
		if (note[3]) {
			if (send4) {
				UART_send(chan_on);
				UART_send(65);
				UART_send(Vel);
				send4 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(65);
			UART_send(0);
			send4 = 1;
		}
		if (note[4]) {
			if (send5) {
				UART_send(chan_on);
				UART_send(67);
				UART_send(Vel);
				send5 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(67);
			UART_send(0);
			send5 = 1;
		}
		if (note[5]) {
			if (send6) {
				UART_send(chan_on);
				UART_send(69);
				UART_send(Vel);
				send6 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(69);
			UART_send(0);
			send6 = 1;
		}
		if (note[6]) {
			if (send7) {
				UART_send(chan_on);
				UART_send(71);
				UART_send(Vel);
				send7 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(71);
			UART_send(0);
			send7 = 1;
		}
		if (note[7]) {
			if (send8) {
				UART_send(chan_on);
				UART_send(72);
				UART_send(Vel);
				send8 = 0;
			}
		} else {
			UART_send(chan_off);
			UART_send(72);
			UART_send(0);
			send8 = 1;
		}
	}
}