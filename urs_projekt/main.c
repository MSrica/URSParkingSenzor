/* TODO
IMPORTANT
doesn't start recording the distance - screen on but no readings, reset needed

KINDA IMPORTANT, HELPS
figuring out every interrupt and explaining it in code/docs

DISPLAY DATA
displaying bars and not values
keeping track of last known value(s) because of the big jumps(false readings)
implementing LED lights somehow
implementing button logic for increasing/decreasing buzzing distance
*/

// defining constants
#define F_CPU 7372800UL
#define triggerPin1 0
#define triggerPin2 1
#define calculationConstant 431.85

// including libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"

// declaring global variables
uint32_t timerOverflow ;
uint32_t count1;
uint32_t count2;
uint16_t distance1;
uint16_t distance2;
uint8_t interrupt0Turn;
uint8_t sensorTurn;
uint8_t buzzingDistance;
char string1[16];
char string2[16];

// interrupt service routines
ISR(INT0_vect){
	if(interrupt0Turn == 0){
		interrupt0Turn = 1;
		TCCR1B |= 1 << CS10;
	}else{
		interrupt0Turn = 0;
		TCCR1B = 0;
		count2 = TCNT1;
		TCNT1 = 0;
	}
}
ISR(TIMER1_OVF_vect){
	timerOverflow++;
}

// initialization
void resetData(){
	timerOverflow = 0;
	count1 = 0;
	count2 = 0;
	distance1 = 0;
	distance2 = 0;
	interrupt0Turn = 0;
	sensorTurn = 0;
	buzzingDistance = 16;
	string1[0] = 0;
	string2[0] = 0;
}
void initializeBuzzer(){
	DDRC = 0xff;
	PORTC = 0xff;
}
void initializeDisplay(){
	DDRD = _BV(4);
	PORTD = 0xFF;
	lcd_init(LCD_DISP_ON);
}
void initializeTriggerPins(){
	DDRA = _BV(0) | _BV(1);
	PORTA = 0x00;
}
void initializeInterruptRegisters(){
	TIMSK = (1 << TOIE1);
	TCCR1A = 0;
	GICR |= 1 << INT0;
	MCUCR |= 1 << ISC00;
	sei();
}
void reinitializeRegisters(){
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
}
void reinitializeCounterValues(){
	count1 = 0;
	distance1 = 0;
}

// display
void setDisplayRegisterValues(){
	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 64;
}
void printValues(){
	lcd_clrscr();
	itoa(distance1,string1,10);
	itoa(distance2,string2,10);
	lcd_gotoxy(0, 0);
	lcd_puts(string1);
	lcd_gotoxy(0, 1);
	lcd_puts(string2);
}

// buzzer
void buzzing(){
	if((distance1 >= buzzingDistance) && (distance2 >= buzzingDistance)) PORTC = 0x01;
	else PORTC = 0x00;
}

// sensor
void shortPulse(uint8_t triggerPin){
	PORTA |= (1 << triggerPin);
	_delay_us(10);
	PORTA &= (~(1 << triggerPin));
}
void risingEdge(){
	TCCR1B = 0x41; // rising edge, no prescaler
	TIFR = 1 << ICF1; //clear input capture flag
}
void fallingEdge(){
	TCNT1 = 0; //timer/counter1 value
	TCCR1B = 0x01; // falling edge, no prescaler
	TIFR = 1 << ICF1; //clear input capture flag
	timerOverflow = 0;
}
void waitingForSignal(){
	while ((TIFR & (1 << ICF1)) == 0);
}

//variables
void setSensorTurn(){
	if(sensorTurn == 0) sensorTurn = 1;
	else sensorTurn = 0;
}
void calculateDistance(){
	if(sensorTurn == 1){
		count1 = ICR1 + (65535 * timerOverflow);
		distance1 = (uint32_t)(count1 / calculationConstant);
	}else distance2 = (uint32_t)(count2 / calculationConstant);
}

void mainLoop(){
	while(1){
		if(sensorTurn == 0){
			setSensorTurn();
			reinitializeRegisters();
			reinitializeCounterValues();

			shortPulse((uint8_t)triggerPin1);
			risingEdge();
			waitingForSignal();
			fallingEdge();
			waitingForSignal();
			
			calculateDistance(sensorTurn);
			
			setDisplayRegisterValues();
			printValues();
			
			_delay_ms(500);
		}else{
			setSensorTurn();
			reinitializeRegisters();
			
			shortPulse((uint8_t)triggerPin2);
			_delay_ms(60);
			
			calculateDistance(sensorTurn);
		}
		
		buzzing();
	}
}

int main(void){	
	resetData();
	initializeBuzzer();
	initializeDisplay();
	initializeTriggerPins();
	initializeInterruptRegisters();
	
	mainLoop();
}