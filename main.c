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
#define col1 PORTD7
#define col2 PORTC2
#define col3 PORTC3
#define col4 PORTC0
#define col5 PORTC1
#define col6 PORTC4
#define col7 PORTC5
#define col8 PORTB0
#define col9 PORTB1
#define col10 PORTB2
#define col11 PORTB3
#define col12 PORTB4
#define col13 PORTB5

//created using Excel spreadsheet and formula y = x ^ 0.5 * 18 - 35
const int velCurve[]={127, 112, 101, 91, 83, 75, 68, 61, 55, 49, 44, 38, 33, 28, 24, 19, 15, 10, 6, 2};


volatile unsigned int overflows = 0;
//volatile int col[2] = {PORTB0, PORTB1, PORTB2, PORTB3};
//volatile int rowFst[2] = {PIND6};
//volatile int rowSnd[2] = {PIND7};

volatile int allcol[22] = {col8, col9, col10, col11, col12, col13, col6, col7, col4, col5, col2, col3,
						  col8, col1, col9, col10, col11, col12, col13, col7, col6, col5};
volatile int allrow[22] = {PIND6, PIND6, PIND6, PIND6, PIND6, PIND6, PIND6, PIND6, PIND6, PIND6, PIND6, PIND6,
						  PIND5, PIND6, PIND5, PIND5, PIND5, PIND5, PIND5, PIND5, PIND5, PIND5};
volatile int uptime[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int note[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int send[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
volatile int off[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
volatile int isFirst[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
volatile int start[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int velocity[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
volatile int notes[11] = {58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48};
volatile unsigned int timer = 0;
	
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
	DDRB |= (1<<DDB4);
	DDRB |= (1<<DDB5);
	
	DDRC |= (1<<DDC0);
	DDRC |= (1<<DDC1);
	DDRC |= (1<<DDC2);
	DDRC |= (1<<DDC3);
	DDRC |= (1<<DDC4);
	DDRC |= (1<<DDC5);
	
	DDRD |= (1<<DDD7);
	//Set Row 6,7 to input
	DDRD &= ~(1<<DDD5);
	DDRD &= ~(1<<DDD6);
	
	//enable overflow interrupts
	TIMSK1 |= (1<<TOIE1); 
	
	//set timer1 prescaler to 1
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~(1<<CS11);
	TCCR1B &= ~(1<<CS12);
	sei();
}

void ReadMatrix() {
	//int count = 0;
	//int i;
	//for (i = 0; i < 22; i+=2) {
		////Upper column on
		//set(column_port, allcol[i]);
		//_delay_us(1000);
		//if (check(row_pin, allrow[i])) {
			//if (overflows - start[count] > 20) {
				//isFirst[count] = 1;
			//}
			//if (isFirst[count]) {
				//isFirst[count] = 0;
				//start[count] = overflows;
			//}
			////Upper column off
			//clear(column_port, allcol[i]);
			//_delay_us(1000);
			////Lower column on
			//set(column_port, allcol[i+1]);
			//_delay_us(1000);
			//if (check(row_pin, allrow[i+1])) {
				//
				//note[count] = 1;
				//int subtract = overflows - start[count];
				////velocity[count] = 120 - (subtract) * 6;
				//if (subtract >= 0 && subtract <= 19) {
					//velocity[count] = velCurve[subtract];
				//} else {
					//velocity[count] = 0;
				//}
				//isFirst[count] = 1;
				////Lower column off
				//clear(column_port, allcol[i+1]);
				//_delay_us(1000);
			//}
		//} else {
			//note[count] = 0;
			////Lower column off
			//clear(column_port, allcol[i+1]);
			//_delay_us(1000);
		//}
		//count++;
		////Upper column off
		//clear(column_port, allcol[i]);
		//_delay_us(1000);
	//}
	
	clear(PORTB, PORTB1);
	set(PORTB, PORTB0);
	_delay_us(1000);
	if (check(PIND, PIND6)) {
		note[0] = 1;
		} else {
		note[0] = 0;
	}
	clear(PORTB, PORTB0);
	
	set(PORTB, PORTB2);
	_delay_us(1000);
	if (check(PIND, PIND6)) {
		note[1] = 1;
		} else {
		note[1] = 0;
	}
	clear(PORTB, PORTB2);
	
	set(PORTB, PORTB4);
	_delay_us(1000);
	if (check(PIND, PIND6)) {
		note[2] = 1;
		} else {
		note[2] = 0;
	}
	clear(PORTB, PORTB4);
	
	set(PORTC, PORTC4);
	_delay_us(1000);
	if (check(PIND, PIND6)) {
		note[3] = 1;
		} else {
		note[3] = 0;
	}
	clear(PORTB, PORTB6);
	
	set(PORTC, PORTC0);
	_delay_us(1000);
	if (check(PIND, PIND6)) {
		note[4] = 1;
		} else {
		note[4] = 0;
	}
	clear(PORTC, PORTC0);
	
	set(PORTC, PORTC2);
	_delay_us(1000);
	if (check(PIND, PIND6)) {
		note[5] = 1;
		} else {
		note[5] = 0;
	}
	clear(PORTC, PORTC2);
	
	set(PORTB, PORTB0);
	_delay_us(1000);
	if (check(PIND, PIND5)) {
		note[6] = 1;
		} else {
		note[6] = 0;
	}
	clear(PORTB, PORTB0);
	
	set(PORTB, PORTB1);
	_delay_us(1000);
	if (check(PIND, PIND5)) {
		note[7] = 1;
		} else {
		note[7] = 0;
	}
	clear(PORTB, PORTB1);
	
	set(PORTB, PORTB3);
	_delay_us(1000);
	if (check(PIND, PIND5)) {
		note[8] = 1;
		} else {
		note[8] = 0;
	}
	clear(PORTD, PORTD5);
	
	set(PORTB, PORTB5);
	_delay_us(1000);
	if (check(PIND, PIND5)) {
		note[9] = 1;
		} else {
		note[9] = 0;
	}
	clear(PORTD, PORTD5);
	
	set(PORTC, PORTC4);
	_delay_us(1000);
	if (check(PIND, PIND5)) {
		note[10] = 1;
		} else {
		note[10] = 0;
	}
	clear(PORTC, PORTC4);
}
int main(void) {
	//Note to future Alex, Spell with s not z
	Initialise();
	UART_init(BAUD_PRESCALER);
	while (1) {
		//timer++;
		ReadMatrix();
		int f;
		for (f=0; f < 11; f++) {
			if (note[f]) {
				if (send[f]) {
					UART_send(chan_on);
					UART_send(notes[f]);
					UART_send(127);
					send[f] = 0;
					off[f] = 1;
					timer = 0;
				}
			}
			if (!note[f]) {
				if (off[f]) {
					uptime[f] = overflows;
					UART_send(chan_off);
					UART_send(notes[f]);
					UART_send(0);
					send[f] = 1;
					off[f] = 0;
				}
			}
		}
		
		//int i;
		//for (i = 0; i < 11; i++) {
			//if (note[i]) {
				//if (send[i]) {
					//UART_send(chan_on);
					//UART_send(notes[i]);
					////UART_send(127);
					//UART_send(velocity[i]);
					//send[i] = 0;
					//off[i] = 1;
				//}
			//}
			//if (!note[i]) {
				//if (off[i]) {
					//uptime[i] = overflows;
					//UART_send(chan_off);
					//UART_send(notes[i]);
					//UART_send(0);
					//send[i] = 1;
					//off[i] = 0;
				//}
			//}
		//}
	}
}

ISR(TIMER1_OVF_vect) {
	overflows++;
}
