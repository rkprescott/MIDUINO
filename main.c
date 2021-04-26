/*
 *  ESE350 MIDI Keyboard project made by some pretty cool folks,
 *	Richard Prescott & Alex GE
 *  
 *  Version: 
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

#define midi_start 250
#define midi_stop 252
#define midi_clk 248

#define chan_on 144
#define chan_off 128

#define set(reg, bit) reg |= (1<<bit)
#define clear(reg, bit) reg &= ~(1<<bit)
#define check(reg, bit) reg & (1<<bit)
#define column_port PORTB
#define row_pin PIND

//created using Excel spreadsheet and formula y = x ^ 0.5 * 18 - 35
const int velCurve[]={127, 112, 101, 91, 83, 75, 68, 61, 55, 49, 44, 38, 33, 28, 24, 19, 15, 10, 6, 2};


volatile int overflows = 0;
//volatile int col[2] = {PORTB0, PORTB1, PORTB2, PORTB3};
//volatile int rowFst[2] = {PIND6};
//volatile int rowSnd[2] = {PIND7};

volatile int allcol[4] = {PORTB0, PORTB1, PORTB2, PORTB3};
volatile int allrow[4] = {PIND6, PIND6, PIND7, PIND7};
volatile int note[2] = {0, 0};
volatile int send[2] = {1, 1};
volatile int isFirst[2] = {1, 1};
volatile int start[2] = {0, 0};
volatile int velocity[2] = {0, 0};
volatile int notes[2] = {60, 59};
	
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
	cli();
	//Set to column to output
	DDRB |= (1<<DDB0);
	DDRB |= (1<<DDB1);
	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
	//Set Row 6,7 to input
	DDRD &= ~(1<<DDD6);
	DDRD &= ~(1<<DDD7);
	
	//enable overflow interrupts
	TIMSK1 |= (1<<TOIE1); 
	
	//set timer1 prescaler to 1
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~(1<<CS11);
	TCCR1B &= ~(1<<CS12);
	sei();
}

void ReadMatrix() {
	int count = 0;
	int i;
	for (i = 0; i < 4; i+=2) {
		//Upper column on
		set(column_port, allcol[i]);
		_delay_us(1);
		if (check(row_pin, allrow[i])) {
			if (overflows - start[count] > 20) {
				isFirst[count] = 1;
			}
			if (isFirst[count]) {
				isFirst[count] = 0;
				start[count] = overflows;
			}
			//Upper column off
			clear(column_port, allcol[i]);
			//Lower column on
			set(column_port, allcol[i+1]);
			_delay_us(1);
			if (check(row_pin, allrow[i])) {
				note[count] = 1;
				int subtract = overflows - start[count];
				//velocity[count] = 120 - (subtract) * 6;
				if (subtract >= 0 && subtract <= 20) {
					velocity[count] = velCurve[subtract];
				} else {
					velocity[count] = 0;
				}
				isFirst[count] = 1;
				//Lower column off
				clear(column_port, allcol[i+1]);
			}
		} else {
			note[count] = 0;
			//Lower column off
			clear(column_port, allcol[i+1]);
		}
		count++;
		//Upper column off
		clear(column_port, allcol[i]);
	}
}
int main(void) {
	//Note to future Alex, Spell with s not z
	Initialise();
	UART_init(BAUD_PRESCALER);
	while (1) {
		ReadMatrix();
		int i;
		for (i = 0; i < 2; i++) {
			if (note[i]) {
				if (send[i]) {
					UART_send(chan_on);
					UART_send(notes[i]);
					UART_send(velocity[i]);
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

ISR(TIMER1_OVF_vect) {
	overflows++;
}