/*
 * Joystick.c
 *
 * Created: 5.4.2016 10:15:39
 *  Author: atom2
 */ 

#include "joystick.h"
#include "common_defs.h"
#include "avr/io.h"

//Inicializace pro Tlaèítka
void TL_Init(void)
{
	//Nastavení pinù DDRx (Log. 1) na výstupní na LED diody
	sbi(LED_PORT_IO, LED_VYP);
	sbi(LED_PORT_IO, LED_OK);
	sbi(LED_PORT_IO, LED_LEFT);
	sbi(LED_PORT_IO, LED_RIGHT);
	
	//Nastavení pinù DDRx (Log. 0) na vstupní na tlaèítka
	cbi(TL_PORT_IO, TL_VYP);
	cbi(TL_PORT_IO, TL_OK);
	cbi(TL_PORT_IO, TL_LEFT);
	cbi(TL_PORT_IO, TL_RIGHT);
	cbi(TL_PORT_IO, TL_XYZ);
	
	//Nastavení Pøerušení pro piny PC0-PC4
	PCMSK2 |= (1 << PCINT16) | (1 << PCINT17) | (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20);
	
	//Nastavení pøerušení pro PORTC
	PCICR |= (1 << PCIE2);
}