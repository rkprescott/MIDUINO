/*
 * VelocitySensing.c
 *
 * Created: 4/12/2021 12:23:17 AM
 * Author : Velocity Testing
 * 
 * This code was not used in the final product, but was an important intermediate
 * step (Milestone). This code shows that the concept of velocity sensing works
 * perfectly on a switch button matrix, which we tried to replicate on our keyboard.
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

volatile int overflows = 0;
volatile int velocity = 50;
volatile int velocity2 = 50;
volatile int note1 = 0;
volatile int note2 = 0;

volatile int send1 = 1;
volatile int send2 = 1;


volatile int isFirst = 1;
volatile int isFirst2 = 1;
volatile int start = 0;
volatile int start2 = 0;

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
	//Set to columns 1,2,8,9 to output
	DDRB |= (1<<DDB0);
	DDRB |= (1<<DDB1);
	DDRB |= (1<<DDB2);
	DDRB |= (1<<DDB3);
	//Set Row 6,7 to input
	DDRD &= ~(1<<DDD6);
	DDRD &= ~(1<<DDD7);
	TIMSK1 |= (1<<TOIE1); //enable overflow interrupts
	
	//set timer1 prescaler to 1
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~(1<<CS11);
	TCCR1B &= ~(1<<CS12);
	sei();
}

void ReadMatrix() {
	//Column 1 on
	PORTB |= (1<<PORTB0);
	_delay_us(1);
	//Row6
	if (PIND & (1<<PIND6)) {
		if (isFirst) {
			isFirst = 0;
			start = overflows;
		}
		//Column 1 off
		PORTB &= ~(1<<PORTB0);
		//Column 2 on
		PORTB |= (1<<PORTB1);
		_delay_us(1);
		if (PIND & (1<<PIND6)) {
			note1 = 1;
			velocity = 120 - (overflows - start) * 6;
			if (velocity < 0) {
				velocity = 0;
			}
			isFirst = 1;
			//Column 2 off
			PORTB &= ~(1<<PORTB1);
		} else {
			note1 = 0;
			//Column 2 off
			PORTB &= ~(1<<PORTB1);
		}
	}
	//Column 1 off
	PORTB &= ~(1<<PORTB0);
	
	//Column 8 on
	PORTB |= (1<<PORTB2);
	_delay_us(1);
	//Row7
	if (PIND & (1<<PIND7)) {
		if (isFirst2) {
			isFirst2 = 0;
			start2 = overflows;
		}
		//Column 8 off
		PORTB &= ~(1<<PORTB2);
		//Column 9 on
		PORTB |= (1<<PORTB3);
		_delay_us(1);
		if (PIND & (1<<PIND7)) {
			note2 = 1;
			velocity2 = 120 - (overflows - start2) * 6;
			if (velocity2 < 0) {
				velocity2 = 0;
			}
			isFirst2 = 1;
			//Column 9 off
			PORTB &= ~(1<<PORTB3);
		} else {
			note2 = 0;
			//Column 9 off
			PORTB &= ~(1<<PORTB3);
		}
	}
	//Column 8 off
	PORTB &= ~(1<<PORTB2);
}

int main(void) {
	//Note to future Alex, Spell with s not z
	Initialise();
	UART_init(BAUD_PRESCALER);
	while (1) {
		ReadMatrix();
		if (note1) {
			if (send1) {
				UART_send(chan_on);
				UART_send(60);
				UART_send(velocity);
				send1 = 0;
			}
			} else {
			UART_send(chan_off);
			UART_send(60);
			UART_send(0);
			send1 = 1;
		}
		if (note2) {
			if (send2) {
				UART_send(chan_on);
				UART_send(62);
				UART_send(velocity2);
				send2 = 0;
			}
			} else {
			UART_send(chan_off);
			UART_send(62);
			UART_send(0);
			send2 = 1;
		}
	}
}

ISR(TIMER1_OVF_vect) {
	overflows++;
}
