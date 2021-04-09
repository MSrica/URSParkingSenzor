// constants
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
#define buttonPin3 (1 << 2)
#define buttonPin4 (1 << 3)
#define diplayBufferSpace 5

#define on 0x00
#define off 0xff

#define calculationConstant 431.85
#define maximumDistance 25-1
#define minimumDistance 5+1

// libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"

// global variables
uint32_t timerOverflow;
uint32_t leftCount;
uint32_t rightCount;
uint16_t leftDistance;
uint16_t rightDistance;
uint8_t interrupt0Turn;
uint8_t sensorTurn;
uint8_t buzzingDistance;
uint8_t handbrake;
uint8_t displayType;
char string1[16];
char string2[16];

// interrupt service routines
ISR(INT0_vect){
	if(interrupt0Turn == 0){
		interrupt0Turn = 1;
		TCCR1B |= 1 << CS10;
	}else{
		interrupt0Turn = 0;
		rightCount = TCNT1;
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
	leftCount = 0;
	rightCount = 0;
	leftDistance = 0;
	rightDistance = 0;
	interrupt0Turn = 0;
	sensorTurn = 0;
	buzzingDistance = 15;
	handbrake = 1;
	displayType = 0;
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
	buttonDDR = buttonPin1 | buttonPin2 | buttonPin3 | buttonPin4;
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
	leftCount = 0;
	leftDistance = 0;
}

// display
void setDisplayRegisterValues(){
	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 96;
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
void displayEmptyBlock(uint8_t x, uint8_t y) {
	lcd_gotoxy(x, y);
	lcd_puts("O");
}
void displayObjectBlock(uint8_t x, uint8_t y) {
	lcd_gotoxy(x, y);
	lcd_puts("#");
}

void printLeftPercentageValues(){
	lcd_gotoxy(0, 0);
	lcd_puts("L:");
	if(leftDistance >= buzzingDistance) for(uint8_t i = 0; i < 10; ++i) displayEmptyBlock(i + diplayBufferSpace, 0);
	else if (leftDistance <= 0.1 * buzzingDistance) for(uint8_t i = 0; i < 10; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
	else if (leftDistance <= 0.2 * buzzingDistance) {
		for(uint8_t i = 0; i < 9; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 9; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.3 * buzzingDistance) {
		for(uint8_t i = 0; i < 8; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 8; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.4 * buzzingDistance) {
		for(uint8_t i = 0; i < 7; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 7; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.5 * buzzingDistance) {
		for(uint8_t i = 0; i < 6; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 6; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.6 * buzzingDistance) {
		for(uint8_t i = 0; i < 5; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 5; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.7 * buzzingDistance) {
		for(uint8_t i = 0; i < 4; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 4; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.8 * buzzingDistance) {
		for(uint8_t i = 0; i < 3; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 3; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= 0.9 * buzzingDistance) {
		for(uint8_t i = 0; i < 2; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 2; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	} else if (leftDistance <= buzzingDistance) {
		for(uint8_t i = 0; i < 1; ++i) displayObjectBlock(i + diplayBufferSpace, 0);
		for(uint8_t i = 1; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 0);
	}
}
void printRightPercentageValues(){
	lcd_gotoxy(0, 1);
	lcd_puts("R:");
	if(rightDistance >= buzzingDistance) for(uint8_t i = 0; i < 10; ++i) displayEmptyBlock(i + diplayBufferSpace, 1);
	else if (rightDistance <= 0.1 * buzzingDistance) for(uint8_t i = 0; i < 10; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
	else if (rightDistance <= 0.2 * buzzingDistance) {
		for(uint8_t i = 0; i < 9; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 9; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.3 * buzzingDistance) {
		for(uint8_t i = 0; i < 8; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 8; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.4 * buzzingDistance) {
		for(uint8_t i = 0; i < 7; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 7; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.5 * buzzingDistance) {
		for(uint8_t i = 0; i < 6; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 6; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.6 * buzzingDistance) {
		for(uint8_t i = 0; i < 5; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 5; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.7 * buzzingDistance) {
		for(uint8_t i = 0; i < 4; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 4; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.8 * buzzingDistance) {
		for(uint8_t i = 0; i < 3; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 3; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= 0.9 * buzzingDistance) {
		for(uint8_t i = 0; i < 2; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 2; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	} else if (rightDistance <= buzzingDistance) {
		for(uint8_t i = 0; i < 1; ++i) displayObjectBlock(i + diplayBufferSpace, 1);
		for(uint8_t i = 1; i < 10; i++) displayEmptyBlock(i + diplayBufferSpace, 1);
	}
}
void printPercentageValues(){
	printLeftPercentageValues();
	printRightPercentageValues();
}

void graphicalClearScreen(){
	for(uint8_t i = 0; i < 16; i++){
		displayEmptyBlock(i, 0);
		displayEmptyBlock(i, 1);	
	}
}
void printLeftGraphicalValues(){
	if(leftDistance <= buzzingDistance * 0.5){
		for(uint8_t i = 0; i < 5; i++){
			displayObjectBlock(i, 0);
			displayObjectBlock(i, 1);
		}
	}else if(leftDistance < buzzingDistance) for(uint8_t i = 0; i < 5; i++) displayObjectBlock(i, 1);
}
void printRightGraphicalValues(){
	if(rightDistance <= buzzingDistance * 0.5){
		for(uint8_t i = 10; i < 16; i++){
			displayObjectBlock(i, 0);
			displayObjectBlock(i, 1);
		}
	}else if(rightDistance < buzzingDistance) for(uint8_t i = 10; i < 16; i++) displayObjectBlock(i, 1);
}
void printMiddleGraphicalValues(){
	if(leftDistance <= buzzingDistance * 0.5 && rightDistance <= buzzingDistance * 0.5){
		for(uint8_t i = 5; i < 10; i++){
			displayObjectBlock(i, 0);
			displayObjectBlock(i, 1);
		}
	}else if(leftDistance < buzzingDistance && rightDistance < buzzingDistance) for(uint8_t i = 5; i < 10; i++) displayObjectBlock(i, 1);
}
void printGraphicalValues(){
	graphicalClearScreen();
	printLeftGraphicalValues();
	printRightGraphicalValues();
	printMiddleGraphicalValues();	
}

void printNumberValues(){
	lcd_clrscr();
	itoa(leftDistance,string1,10);
	itoa(rightDistance,string2,10);
	lcd_gotoxy(0, 0);
	lcd_puts(string1);
	lcd_gotoxy(0, 1);
	lcd_puts(string2);
}

void printValues(){
	lcd_clrscr();
	
	if(displayType == 0) printGraphicalValues();
	else if(displayType == 1) printPercentageValues();
	else printNumberValues();
}

// buzzing
void buzzing(){
	if((leftDistance >= buzzingDistance) && (rightDistance >= buzzingDistance)) buzzerPORT = 0x01;
	else {
		if ((leftDistance <= (0.2 * buzzingDistance)) || (rightDistance <= (0.2 * buzzingDistance))) buzzerPORT = 0x00;
		else {
			for(int i = 0; i < 10; i++){
				if(i % 2 == 0) buzzerPORT = 0x00;
				else buzzerPORT =  0x01;
				if ((leftDistance <= (0.4 * buzzingDistance)) || (rightDistance <= (0.4 * buzzingDistance))) _delay_ms(35);
				else if ((leftDistance <= (0.6 * buzzingDistance)) || (rightDistance <= (0.6 * buzzingDistance))) _delay_ms(70);
				else if ((leftDistance <= (0.8 * buzzingDistance)) || (rightDistance <= (0.8 * buzzingDistance))) _delay_ms(105);
				else _delay_ms(140);
			}
		}
	}
}

// buttons
void addToBuzzingDistance(){
	char buzzingString[16];
	setDisplayRegisterValues();
	lcd_clrscr();
	
	if(buzzingDistance <= maximumDistance){
		buzzingDistance += 1;
		itoa(buzzingDistance,buzzingString,10);
		lcd_puts("Range increased");
	}else lcd_puts("Maximum distance");
	
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
	}else lcd_puts("Minimum distance");
	
	lcd_gotoxy(7, 1);
	lcd_puts(buzzingString);
	_delay_ms(500);
}
void changeDisplayType(){
	lcd_clrscr();
	
	lcd_gotoxy(1,0);
	lcd_puts("Display changed");
	
	displayType = (displayType + 1) % 3;
	_delay_ms(500);
}
void handbrakePull() {
	lcd_clrscr();
	
	if (handbrake == 0)	{
		lcd_gotoxy(2,0);
		lcd_puts("Handbrake up");
	}
	else {
		lcd_gotoxy(1,0);
		lcd_puts("Handbrake down");
	}
	handbrake = (handbrake + 1) % 2;
	_delay_ms(500);
}
void buttonPress(){
	if(bit_is_clear(PINB, 0)) addToBuzzingDistance();
	else if(bit_is_clear(PINB, 1)) subtractFromBuzzingDistance();
	else if(bit_is_clear(PINB, 2)) changeDisplayType();
	else if(bit_is_clear(PINB, 3)) handbrakePull();
}

// sensor
void shortPulse(uint8_t triggerPin){
	triggerPORT |= triggerPin;
	_delay_us(10);
	triggerPORT &= (~triggerPin);
}
void risingEdge(){
	TCCR1B = (1 << ICES1) | (1 << CS10); // rising edge, no prescaler
	TIFR = 1 << ICF1; //clear input capture flag
}
void fallingEdge(){
	TCNT1 = 0; //timer/counter1 value
	TCCR1B = 1 << CS10; // falling edge, no prescaler
	TIFR = 1 << ICF1; //clear input capture flag
	timerOverflow = 0;
}
void waitingForSignal(){
	while ((TIFR & (1 << ICF1)) == 0);
}

//variables
void setSensorTurn(){
	sensorTurn = (sensorTurn + 1) % 2;
}
void calculateDistance(){
	if(sensorTurn == 0){
		leftCount = ICR1 + (65535 * timerOverflow);
		leftDistance = (uint32_t)(leftCount / calculationConstant);
	}else rightDistance = (uint32_t)(rightCount / calculationConstant);
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
		printValues();
		
		if (handbrake == 0) buzzing();
		
		_delay_ms(100);
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