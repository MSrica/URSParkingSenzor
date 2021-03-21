#define F_CPU 7372800UL
#define Trigger_pin	 PA0				
#define Trigger_pin1 PA1			

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include "lcd.h"						

int TimerOverflow = 0;

ISR(TIMER1_OVF_vect){
	TimerOverflow++;					
}

static volatile int pulse = 0;
static volatile int i = 0;

ISR(INT0_vect){
	if(i == 0){
		TCCR1B |= 1<<CS10;
		i = 1;
	}else{
		TCCR1B = 0;
		pulse = TCNT1;
		TCNT1 = 0;
		i = 0;
	}
}

uint8_t cnt = 0;
uint8_t initCnt = 0;

int main(void){
	DDRC = 0xff;
	PORTC = 0xff;

	DDRD = _BV(4);
	lcd_init(LCD_DISP_ON);

	char string[16];						
	uint32_t count;
	uint16_t distance;
	uint32_t count_a = 0;
	char show_a[16];

	DDRA = _BV(0) | _BV(1); 
	PORTA = 0x00;								
	PORTD = 0xFF;							
								
	TIMSK = (1 << TOIE1);						
	TCCR1A = 0;								
	GICR |= 1 << INT0;
	MCUCR |= 1 << ISC00;
	sei();

	while(1){
		if(cnt == 0){
			if(initCnt > 10){
				cnt = 1;
			}
			initCnt += 1;
			
			TCCR1A = 0;
			count = 0;
			distance = 0;

			PORTA |= (1 << Trigger_pin);
			_delay_us(10);
			PORTA &= (~(1 << Trigger_pin));

			TCNT1 = 0;
			TCCR1B = 0x41;
			TIFR = 1 << ICF1;
			TIFR = 1 << TOV1;

			while ((TIFR & (1 << ICF1)) == 0);
			TCNT1 = 0;
			TCCR1B = 0x01;
			TIFR = 1 << ICF1;
			TIFR = 1 << TOV1;
			TimerOverflow = 0;

			while ((TIFR & (1 << ICF1)) == 0);
			count = ICR1 + (65535 * TimerOverflow);
			distance = (uint32_t)count / 431.85;
			
			TCCR1A = _BV(COM1B1) | _BV(WGM10);
			TCCR1B = _BV(WGM12) | _BV(CS11);
			OCR1B = 64;

			lcd_clrscr();
			itoa(distance,string,10);
			itoa(count_a,show_a,10);
			lcd_gotoxy(0, 0);
			lcd_puts(string);
			lcd_gotoxy(0, 1);
			lcd_puts(show_a);
			_delay_ms(200);
		}else if(initCnt > 10){
			cnt = 0;
			TCCR1A = 0;
			TCCR1B = 0;
			TCNT1 = 0;
			PORTA |= (1 << Trigger_pin1);
			_delay_us(10);
			PORTA &= (~(1 << Trigger_pin1));
			_delay_ms(60);
			count_a = pulse/510;
		}
		
		if(distance >= 16){
			PORTC = 0x01;
		}else{
			PORTC = 0x00;
		}
	}
}