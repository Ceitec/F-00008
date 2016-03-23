/*
 * IncFile1.h
 *
 * Created: 23.3.2016 9:57:39
 *  Author: atom2
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#include <avr/io.h>

//ISR(ADC_vect);
void InitADC();
uint16_t ReadADC(uint8_t ADCchannel);


#endif /* INCFILE1_H_ */