/*
 * ShiftRegister.c
 *
 * Created: 5.4.2016 9:20:39
 *  Author: atom2
 */ 

#include "ShiftRegister.h"
#include "common_defs.h"
#include <avr/io.h>

/* Nastavení pøeddìlièky 256
	Z výpoètu - 1/(12000000/256) = 21,33333 us
	Proto aby pøerušení vzniklo vždy v 1ms tak
	musíme nastavit TCNT dle 1ms / 21,3333us = ~46,875
	zaokrouhleno na 47 takže OCR0A nastavíme 256 - 47
	TCNT0 = 209 nebo OCR0A = 46
	
*/

void Init_Shift_Register (void)
{
	// Delicka 256
	TCCR0B |= (1 << CS02) | (1 << CS00);
	// Nastavení comparatoru na 0
	TCNT0 = 0;
	// Nastavení hodnoty registru A pro pøerušení
	OCR0A = 46;
	// Porovnání Comparatoru s registrem A
	TIMSK0 |= (1 << OCIE0A);
		
	// Nastavení pinù na V/V
	sbi(Display_DDR, Display_SER);		//Vystup Data
	sbi(Display_DDR, Display_SCK);		//Vystup Shift register clock input (Pri nábìzné hranì se data nahrají do registru)
	sbi(Display_DDR, Display_SCL);		//Vystup
	sbi(Display_DDR, Display_RCK);		//Vystup Data storage clock input (Po nahrani všech x bitù se po nabìzne hrane nahraji do registru)
	sbi(Display_DDR, Display_G);		//Vystup (Pri logicke 0 dojde k zobrazeni na segmenty)

	// Nastavení pinu G na LOG.0
	//Permanentní nastavení pinù
	cbi(Display_PORT, Display_G);
	sbi(Display_PORT, Display_SCL);
}

