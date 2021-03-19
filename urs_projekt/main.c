#define F_CPU 7372800UL
#define Trigger_pin1 PA0							/* Trigger pin */
#define Trigger_pin2 PA1							/* Trigger pin */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"									/* Include LCD header file */

uint8_t cnt = 0;
uint32_t TimerOverflow = 0;
char string[14];								/* Variables init */
char string1[14];
uint32_t distance = 0;

ISR(TIMER0_COMP_vect){
	TimerOverflow++;								/* Increment Timer Overflow count */	
}

ISR(INT0_vect){	
	distance = (TimerOverflow * 3) / 2;
	TimerOverflow = 0;
	
	lcd_clrscr();
	if(cnt == 1){
		//itoa(distance, string, 10);
		dtostrf(distance, 4, 2, string);
	}
	if(cnt == 0){
		//itoa(distance, string1, 10);
		dtostrf(distance, 4, 2, string1);
	}
	lcd_gotoxy(0, 0);
	lcd_puts(string);
	lcd_gotoxy(0, 1);
	lcd_puts(string1);
	
	TCNT0 = 0;
	TCCR0 |= _BV(CS00);
}

int main(void){
	DDRC = 0xff;									/* Buzzer port */
	PORTC = 0xff;
	
	DDRD = _BV(4);					/* LCD port */
	lcd_init(LCD_DISP_ON);							/* LCD init */
	
	//DDRA = _BV(0) | _BV(1); 									/* Make trigger pin as output */
	PORTD = 0xFF;									/* Turn on Pull-up */
					
	TCCR0 = _BV(WGM01) | _BV(CS00);
	OCR0 = 36;
	
	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 64;
													/* Enable global interrupt */
	TIMSK = (1 << OCIE0);							/* Enable Timer1 overflow interrupts */
	GICR = _BV(INT0);
	MCUCR = _BV(ISC00);
	sei();		


	while(1){
//---------------------------------------------------------------------------------------------------------
		if(cnt == 0){
			//cnt = 1;
			DDRA = _BV(0);
			PORTA = 0x00;
			PORTA |= (1 << Trigger_pin1);
			_delay_us(10);
			PORTA &= (~(1 << Trigger_pin1));
		}else{
			cnt = 0;
			DDRA = _BV(1);
			PORTA = 0x00;
			PORTA |= (1 << Trigger_pin2);
			_delay_us(10);
			PORTA &= (~(1 << Trigger_pin2));
		}
		_delay_ms(65);

		/*if(distance >= 16){
			PORTC = 0x01;
		}else{
			PORTC = 0x00;
		}*/
	}
	
	
	
}