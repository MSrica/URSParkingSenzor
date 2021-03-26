/* TODO
IMPORTANT
increase, decrease buzzing distace and logic for printin "O" values
docs

DISPLAY DATA
displaying bars and not values
keeping track of last known value(s) because of the big jumps(false readings)
*/

// defining constants
#define F_CPU 7372800UL

#define displayPORT PORTD
#define displayDDR DDRD
#define diplayPin (1 << 4)
#define buzzerPORT PORTC
#define buzzerDDR DDRC
#define buzzerPin (1 << 0)
#define triggerPORT PORTA
#define triggerDDR DDRA
#define triggerPin1 (1 << 0)
#define triggerPin2 (1 << 1)
#define buttonPORT PORTB
#define buttonDDR DDRB
#define buttonPin1 (1 << 0)
#define buttonPin2 (1 << 1)

#define on 0x00
#define off 0xff

#define calculationConstant 431.85
#define maximumDistance 25-1
#define minimumDistance 5+1

// including libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"

// declaring global variables
uint32_t timerOverflow;
uint32_t count1;
uint32_t count2;
uint16_t distance1;
uint16_t distance2;
uint8_t interrupt0Turn;
uint8_t sensorTurn;
uint8_t buzzingDistance;
uint8_t startBuzzing;
char string1[16];
char string2[16];

// interrupt service routines
ISR(INT0_vect){
	if(interrupt0Turn == 0){
		interrupt0Turn = 1;
		TCCR1B |= 1 << CS10;
	}else{
		interrupt0Turn = 0;
		count2 = TCNT1;
		TCCR1B = 0;
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
	buzzingDistance = 15;
	startBuzzing = 0;
	string1[0] = 0;
	string2[0] = 0;
}
void initializeBuzzer(){
	buzzerDDR = buzzerPin;
	buzzerPORT = off;
}
void initializeDisplay(){
	displayDDR = diplayPin;
	displayPORT = on;
	lcd_init(LCD_DISP_ON);
}
void initializeTriggerPins(){
	triggerDDR = triggerPin1 | triggerPin2;
	triggerPORT = on;
}
void initializeButtons(){
	buttonDDR = buttonPin1 | buttonPin2;
	buttonPORT = off;
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
	OCR1B = 96;
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
void splashScreen(){
	_delay_ms(200);
	
	setDisplayRegisterValues();
	lcd_clrscr();
	lcd_gotoxy(2,0);
	lcd_puts("URS projekt");
	lcd_gotoxy(1,1);
	lcd_puts("Parking senzor");
	
	_delay_ms(1500);
}
void displayBlock(uint8_t x, uint8_t y) {
	lcd_gotoxy(x, y);
	lcd_puts("O");
}
void printOValues(){
	lcd_clrscr();
	
	if(distance1 <= 15) {
		for(uint8_t i = 0; i < 5; ++i)
			displayBlock(i, 1);
	}
	if(distance1 <= 5) {
		for(uint8_t i = 0; i< 5; ++i) {
			displayBlock(i, 1);
			displayBlock(i, 0);
		}
	}
	
	if(distance1 <= 15 && distance2 <= 15) {
		for(uint8_t i = 5; i < 11; ++i)
			displayBlock(i, 1);
	}
	if(distance1 <= 5 && distance2 <= 5) {
		for(uint8_t i = 5; i< 11; ++i) {
			displayBlock(i, 1);
			displayBlock(i, 0);
		}
	}
	
	if(distance2 <= 15) {
		for(uint8_t i = 11; i < 16; ++i)
			displayBlock(i, 1);
	}
	if(distance2 <= 5) {
		for(uint8_t i = 11; i < 16; ++i) {
			displayBlock(i, 1);
			displayBlock(i, 0);
		}
	}
}

// buzzer and buttons
void setStartBuzzing(){
	if(startBuzzing == 0) startBuzzing = 1;
}
void buzzing(){
	if(startBuzzing == 1){
		if((distance1 >= buzzingDistance) && (distance2 >= buzzingDistance)) buzzerPORT = 0x01;
		else buzzerPORT = 0x00;
	}
}
void addToBuzzingDistance(){
	char buzzingString[16];
	setDisplayRegisterValues();
	lcd_clrscr();
	
	if(buzzingDistance <= maximumDistance){
		buzzingDistance += 1;
		itoa(buzzingDistance,buzzingString,10);
		lcd_puts("Range increased");
	}else{
		strncpy(buzzingString, "Maximum distance", 16);
	}
	
	lcd_gotoxy(7, 1);
	lcd_puts(buzzingString);
	_delay_ms(500);
}
void subtractFromBuzzingDistance(){
	char buzzingString[16];
	setDisplayRegisterValues();
	lcd_clrscr();
	
	if(buzzingDistance >= minimumDistance){
		buzzingDistance -= 1;
		itoa(buzzingDistance,buzzingString,10);
		lcd_puts("Range decreased");
	}else{
		strncpy(buzzingString, "Minimum distance", 16);
	}
	
	lcd_gotoxy(7, 1);
	lcd_puts(buzzingString);
	_delay_ms(500);
}
void buttonPress(){
	if(bit_is_clear(PINB, 0)) addToBuzzingDistance();
	else if(bit_is_clear(PINB, 1)) subtractFromBuzzingDistance();
}

// sensor
void shortPulse(uint8_t triggerPin){
	triggerPORT |= triggerPin;
	_delay_us(10);
	triggerPORT &= (~triggerPin);
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
	if(sensorTurn == 0){
		count1 = ICR1 + (65535 * timerOverflow);
		distance1 = (uint32_t)(count1 / calculationConstant);
	}else distance2 = (uint32_t)(count2 / calculationConstant);
}

void mainLoop(){
	while(1){
		buttonPress();
		
		if(sensorTurn == 0){
			setSensorTurn();
			reinitializeRegisters();
			
			shortPulse(triggerPin2);
			_delay_ms(60);
			
			calculateDistance(sensorTurn);
		}else{
			setSensorTurn();
			setStartBuzzing();
			reinitializeRegisters();
			reinitializeCounterValues();

			shortPulse(triggerPin1);
			risingEdge();
			waitingForSignal();
			fallingEdge();
			waitingForSignal();
			
			calculateDistance(sensorTurn);
		}
		
		setDisplayRegisterValues();
		
		//printOValues();
		printValues();
		
		//buzzing();
		
		_delay_ms(500);
	}
}

int main(void){	
	resetData();
	initializeBuzzer();
	initializeButtons();
	initializeDisplay();
	initializeTriggerPins();
	initializeInterruptRegisters();
	
	splashScreen();
	
	mainLoop();
}