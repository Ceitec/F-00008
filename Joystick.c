/*
 * Joystick.c
 *
 * Created: 5.4.2016 10:15:39
 *  Author: atom2
 */ 

#include "joystick.h"
#include "common_defs.h"
#include "avr/io.h"

//Inicializace pro Tla��tka
void TL_Init(void)
{
	//Nastaven� pin� DDRx (Log. 1) na v�stupn� na LED diody
	sbi(LED_PORT_IO, LED_VYP);
	sbi(LED_PORT_IO, LED_OK);
	sbi(LED_PORT_IO, LED_LEFT);
	sbi(LED_PORT_IO, LED_RIGHT);
	
	//Nastaven� pin� DDRx (Log. 0) na vstupn� na tla��tka
	cbi(TL_PORT_IO, TL_VYP);
	cbi(TL_PORT_IO, TL_OK);
	cbi(TL_PORT_IO, TL_LEFT);
	cbi(TL_PORT_IO, TL_RIGHT);
	cbi(TL_PORT_IO, TL_XYZ);
	
	//Nastaven� P�eru�en� pro piny PC0-PC4
	PCMSK2 |= (1 << PCINT16) | (1 << PCINT17) | (1 << PCINT18) | (1 << PCINT19) | (1 << PCINT20);
	
	//Nastaven� p�eru�en� pro PORTC
	PCICR |= (1 << PCIE2);
}