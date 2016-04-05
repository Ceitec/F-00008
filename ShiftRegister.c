/*
 * ShiftRegister.c
 *
 * Created: 5.4.2016 9:20:39
 *  Author: atom2
 */ 

#include "ShiftRegister.h"
#include "common_defs.h"
#include <avr/io.h>

/* Nastaven� p�edd�li�ky 256
	Z v�po�tu - 1/(12000000/256) = 21,33333 us
	Proto aby p�eru�en� vzniklo v�dy v 1ms tak
	mus�me nastavit TCNT dle 1ms / 21,3333us = ~46,875
	zaokrouhleno na 47 tak�e OCR0A nastav�me 256 - 47
	TCNT0 = 209 nebo OCR0A = 46
	
*/

void Init_Shift_Register (void)
{
	// Delicka 256
	TCCR0B |= (1 << CS02) | (1 << CS00);
	// Nastaven� comparatoru na 0
	TCNT0 = 0;
	// Nastaven� hodnoty registru A pro p�eru�en�
	OCR0A = 46;
	// Porovn�n� Comparatoru s registrem A
	TIMSK0 |= (1 << OCIE0A);
		
	// Nastaven� pin� na V/V
	sbi(Display_DDR, Display_SER);		//Vystup Data
	sbi(Display_DDR, Display_SCK);		//Vystup Shift register clock input (Pri n�b�zn� hran� se data nahraj� do registru)
	sbi(Display_DDR, Display_SCL);		//Vystup
	sbi(Display_DDR, Display_RCK);		//Vystup Data storage clock input (Po nahrani v�ech x bit� se po nab�zne hrane nahraji do registru)
	sbi(Display_DDR, Display_G);		//Vystup (Pri logicke 0 dojde k zobrazeni na segmenty)

	// Nastaven� pinu G na LOG.0
	//Permanentn� nastaven� pin�
	cbi(Display_PORT, Display_G);
	sbi(Display_PORT, Display_SCL);
}

