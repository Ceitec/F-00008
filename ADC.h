/*
 * IncFile1.h
 *
 * Created: 23.3.2016 9:57:39
 *  Author: atom2
 */ 


#ifndef _ADC_H_
#define _ADC_H_

	#include <avr/io.h>

	void InitADC(void);
	uint16_t ReadADC(uint8_t ADCchannel);


#endif /* _ADC_H_ */