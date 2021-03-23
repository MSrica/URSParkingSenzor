/* TODO
IMPORTANT
sometimes it doesn't start recording the distance - screen on but no readings, needed longer reset or disconnecting PD2 and connecting it after turning on board
overflow - it just stops after some time
displaying bars and not values

KINDA IMPORTANT, HELPS
figuring out every interrupt and explaining it in code/docs

POLISHING
keeping track of last known value(s) because of the big jumps(false readings)
implementing LED lights somehow
implementing button logic for increasing/decreasing buzzing distance
*/


// defining constants
#define F_CPU 7372800UL
#define Trigger_pin1 PA0				
#define Trigger_pin2 PA1			

// including libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"						

// declaring global variables
uint32_t timerOverflow = 0;
uint32_t count2 = 0;
uint8_t interrupt0Turn = 0;
uint8_t sensorTurn = 0;
uint8_t buzzingDistance = 16;

// timer 2 routine
ISR(TIMER1_OVF_vect){
	timerOverflow++;					
}

// timer 0 routine
ISR(INT0_vect){
	if(interrupt0Turn == 0){
		TCCR1B |= 1 << CS10;
		interrupt0Turn = 1;
	}else{
		TCCR1B = 0;
		count2 = TCNT1;
		TCNT1 = 0;
		interrupt0Turn = 0;
	}
}

int main(void){
	// declaring variables
	uint32_t count1 = 0;
	uint16_t distance1 = 0;
	uint16_t distance2 = 0;
	char string1[16];
	char string2[16];
	
	// buzzer initialization
	DDRC = 0xff;
	PORTC = 0xff;

	// display initialization
	DDRD = _BV(4);
	PORTD = 0xFF;
	lcd_init(LCD_DISP_ON);

	// trigger pins setup
	DDRA = _BV(0) | _BV(1); 
	PORTA = 0x00;															
			
	// interrupt registers initialization			
	TIMSK = (1 << TOIE1);						
	TCCR1A = 0;								
	GICR |= 1 << INT0;
	MCUCR |= 1 << ISC00;
	sei();

	// main loop
	while(1){
		if(sensorTurn == 0){
			// setting up next sensor reading
			sensorTurn = 1;
			
			// variables and registers reinitialization
			TCCR1A = 0;
			count1 = 0;
			distance1 = 0;

			// short pulse used for starting the ranging
			PORTA |= (1 << Trigger_pin1);
			_delay_us(10);
			PORTA &= (~(1 << Trigger_pin1));

			TCNT1 = 0;									// NE ZNAM OBJASNIT ODOVUDA
			TCCR1B = 0x41;								// UKLJUCUJUCI INTERRUPTE
			TIFR = 1 << ICF1;
			TIFR = 1 << TOV1;

			while ((TIFR & (1 << ICF1)) == 0);
			TCNT1 = 0;
			TCCR1B = 0x01;
			TIFR = 1 << ICF1;
			TIFR = 1 << TOV1;
			timerOverflow = 0;

			while ((TIFR & (1 << ICF1)) == 0);
			count1 = ICR1 + (65535 * timerOverflow);
			distance1 = (uint32_t)count1 / 431.85;		// DO OVUDA
			
			// display register values
			TCCR1A = _BV(COM1B1) | _BV(WGM10);
			TCCR1B = _BV(WGM12) | _BV(CS11);
			OCR1B = 64;

			// printing values to display
			lcd_clrscr();
			itoa(distance1,string1,10);
			itoa(distance2,string2,10);
			lcd_gotoxy(0, 0);
			lcd_puts(string1);
			lcd_gotoxy(0, 1);
			lcd_puts(string2);
			_delay_ms(500);
		}else{
			// setting up next sensor reading
			sensorTurn = 0;
			
			// registers reinitialization
			TCCR1A = 0;
			TCCR1B = 0;
			TCNT1 = 0;
			
			// short pulse used for starting the ranging
			PORTA |= (1 << Trigger_pin2);
			_delay_us(10);
			PORTA &= (~(1 << Trigger_pin2));
			
			// securing 60 ms pause in order to prevent noise
			_delay_ms(60);
			
			// calculating the distance
			distance2 = count2 / 431.85;
		}
		
		// buzzing the buzzer
		if(distance1 >= buzzingDistance && distance2 >= buzzingDistance){
			PORTC = 0x01;
		}else{
			PORTC = 0x00;
		}
	}
}