void display_block(uint8_t x, uint8_t y) {
	lcd.gotoxy(x, y);
	lcd.write(0xff);	//nisan sigura kako ispisat onu kockicu pa ćemo to tribat isprovat
}

void blinkingLeds() {
	//whichever port ih stavimo
	PORTB = 0b00000000;
	delay_ms(100);
	PORTB = 0b11111111;
	delay_ms(100);
}

int main() {
	
	/*
	consts:
			LEFT_mid --> vrednost livog senzora kad je unutar udaljenosti koju želimo očitavat (triggers the first line)
			LEFT_close --> vrednost kada je preblizu (triggers the both lines)
			RIGHT_mid
			RIGHT_close
	[range] --> moramo odredit 
	*/
	
	/*
		ako podilimo display na tri dela -->
			  0 1 2 3 4 5 6 7   8 9 10 
			[ 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 ]	0/x
			[ 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 ]	1/y
		
		ovaj prvi block je kad su samo podiljeni, znači liva polovica je za livi senzora
		desna polovica displaya za desni senzor
		pali se prvo "vanjska" (y) linija, pa kad je preblizu "unutarnja"(x) linija uz vanjsku
	*/
	if(LEFT_mid in [range]) {
		for(uint8_t i = 0; i< 8; ++i) {	// x element 0-7
			display_block(i, 1);	//"vanjski red" aka y
		}
	}
	if(LEFT_close in [range]) {
		for(uint8_t i = 0; i< 8; ++i) {	// x element 0-7
			display_block(i, 1);	//"vanjski red" aka y
			display_block(i, 0);	//"unutaranji red" aka x
		}
		//dodat buzzer code
		blinkingLeds();
	}
	if(RIGHT_mid in [range]) {
		for(uint8_t i = 8; i< 16; ++i) {	// x element 8-16
			display_block(i, 1);	//"vanjski red" aka y
		}
	}
	if(RIGHT_close in [range]) {
		for(uint8_t i = 8; i< 16; ++i) {	// x element 8-16
			display_block(i, 1);	//"vanjski red" aka y
			display_block(i, 0);	//"unutaranji red" aka x
		}
		//dodat buzzer code
		blinkingLeds();
	}
	
	/*
		ako podilimo display na tri dela -->
			  0 1 2 3 4   5 6 7 8 9 10  
			[ 0 0 0 0 0 | 0 0 0 0 0 0 | 0 0 0 0 0 ]	0/x
			[ 0 0 0 0 0 | 0 0 0 0 0 0 | 0 0 0 0 0 ]	1/y
		
		livin segmentom [y = 0/1, x = [0,4]] upravlja samo livi senzor 
		srednjim segmentom [y = 0/1, x = [5, 10]] upravljaju oba senzora (moraju oba očitavat da prikazuje block) 
		desnin segmentom [y = 0/1, x = [11, 16]] upravlja samo desni senzor 
	*/
	
	if(LEFT_mid in [range]) {
		for(uint8_t i = 0; i< 5; ++i) {	// x element 0-7
			display_block(i, 1);	//"vanjski red" aka y
		}
	}
	if(LEFT_close in [range]) {
		for(uint8_t i = 0; i< 5; ++i) {	// x element 0-7
			display_block(i, 1);	//"vanjski red" aka y
			display_block(i, 0);	//"unutaranji red" aka x
		}
		//dodat buzzer code
		blinkingLeds();
	}
	if(LEFT_mid in [range] && RIGHT_mid in [range]) {
		for (uint8_t i = 5; i < 11; ++i) {
			display_block(i, 1);
		}
	}
	if(LEFT_close in [range] && RIGHT_close in [range]) {
		for (uint8_t i = 5; i < 11; ++i) {
			display_block(i, 1);
			display_block(i, 0);
		}
		//dodat buzzer code
		blinkingLeds();
	}
	if(RIGHT_mid in [range]) {
		for(uint8_t i = 11; i< 16; ++i) {	// x element 8-16
			display_block(i, 1);	//"vanjski red" aka y
		}
	}
	if(RIGHT_close in [range]) {
		for(uint8_t i = 11; i< 16; ++i) {	// x element 8-16
			display_block(i, 1);	//"vanjski red" aka y
			display_block(i, 0);	//"unutaranji red" aka x
		}
		//dodat buzzer code
		blinkingLeds();
	}	
	
}