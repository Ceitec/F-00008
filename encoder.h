//Fuses LOW FUSE BYTE: 0xCF, HIGH FUSE BYTE: 0x18, LOCKOPT BYTE: 0x3F. 

#ifndef	encoder_h
#define	encoder_h
#include <avr/io.h>
//_________________________________________
//Definice pinù
#define ENC_PORT PINA	//	Port kde jsou Encodery
#define ENC_CTL	DDRA	//	Pro nastevení Pull-up rezistorù
#define ENC_WR	PORTA	//	Zápis do portu Encoderu
#define ENC_RD	PINA	//	Ètení portu Encoderu
#define ENC_A PA0		//	Pin A Encoderu
#define ENC_B PA1		//	Pin B Encoderu

#define PCIE0_Part		PCIE0
#define PCINT_Vect_Part	PCINT0_vect
#define PCMSK_Registr	PCMSK0
#define PCINT_ENC_A		PCINT0
#define PCINT_ENC_B		PCINT1
//______________________


void ENC_InitEncoder(void);
void ENC_Intterupt_Set(void);

#endif  //encoder_h



