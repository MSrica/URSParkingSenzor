#define F_CPU 7372800UL
#define Trigger_pin	PA0							/* Trigger pin */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"									/* Include LCD header file */

int TimerOverflow = 0;

ISR(TIMER1_OVF_vect){
	TimerOverflow++;								/* Increment Timer Overflow count */
}


int main(void){
	DDRC = 0xff;									/* Buzzer port */
	
	DDRD = _BV(4);									/* LCD port */
	lcd_init(LCD_DISP_ON);							/* LCD init */
	
	char string[14];								/* Variables init */
	long count;
	double distance;
	
	DDRA = _BV(0); 									/* Make trigger pin as output */
	PORTD = 0xFF;									/* Turn on Pull-up */
	
	sei();											/* Enable global interrupt */
	TIMSK = (1 << TOIE1);							/* Enable Timer1 overflow interrupts */
	TCCR1A = 0;										/* Set all bit to zero Normal operation */

	while(1){
		TCCR1A = 0;									/* Set all bit to zero Normal operation */
		count = 0;
		distance = 0;
		
		PORTA |= (1 << Trigger_pin);				/* Give 10us trigger pulse on trig. pin to HC-SR04 */
		_delay_us(10);
		PORTA &= (~(1 << Trigger_pin));
		
		TCNT1 = 0;									/* Clear Timer counter */
		TCCR1B = 0x41;								/* Capture on rising edge, No prescaler*/
		TIFR = 1 << ICF1;							/* Clear ICP flag (Input Capture flag) */
		TIFR = 1 << TOV1;							/* Clear Timer Overflow flag */

		/* Calculate width of Echo by Input Capture (ICP) */
		while ((TIFR & (1 << ICF1)) == 0);			/* Wait for rising edge */
		TCNT1 = 0;									/* Clear Timer counter */
		TCCR1B = 0x01;								/* Capture on falling edge, No prescaler */
		TIFR = 1 << ICF1;							/* Clear ICP flag (Input Capture flag) */
		TIFR = 1 << TOV1;							/* Clear Timer Overflow flag */
		TimerOverflow = 0;							/* Clear Timer overflow count */

		while ((TIFR & (1 << ICF1)) == 0);			/* Wait for falling edge */
		count = ICR1 + (65535 * TimerOverflow);		/* Take count */
		/* 8MHz Timer freq, sound speed =343 m/s */
		distance = (double)count / 431.85;
		
		if(distance >= 16){
			PORTC = 0x01;
			}else{
			PORTC = 0x00;
		}
		
		
		TCCR1A = _BV(COM1B1) | _BV(WGM10);
		TCCR1B = _BV(WGM12) | _BV(CS11);
		OCR1B = 64;
		
		
		dtostrf(distance, 4, 2, string);			/* distance to string */
		strcat(string, " cm   ");					/* Concat unit i.e.cm */
		lcd_gotoxy(0, 0);
		lcd_puts(string);
		_delay_ms(250);
	}
}