/*
 * CFile1.c
 *
 * Created: 23.3.2016 9:57:06
 *  Author: atom2
 */ 

#include "ADC.h"



void InitADC(void)
{
	// Select Vref=AVcc
	ADMUX |= (1 << REFS0);
	//set prescaller to 128 and enable ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADC;
}