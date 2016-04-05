/*
 * Joystick.h
 *
 * Created: 5.4.2016 10:16:11
 *  Author: atom2
 */ 

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

//Definování Tlaèítek k externí desce
#define	TL_PORT_IN	PINC
#define TL_PORT_OUT	PORTC
#define TL_PORT_IO	DDRC
#define TL_MASK		0x1F
#define TL_VYP		PC0
#define TL_LEFT		PC1
#define TL_RIGHT	PC2
#define TL_OK		PC3
#define TL_XYZ		PC4

//Definování LED diod u tlaèítek k externí desce
#define LED_PORT_IN		PIND
#define LED_PORT_OUT	PORTD
#define LED_PORT_IO		DDRD
#define LED_VYP			PD3
#define LED_LEFT		PD5
#define LED_RIGHT		PD6
#define LED_OK			PD7

//Definování hlavièek funkcí
void TL_Init(void);
ISR(PCINT2_vect);


#endif /* JOYSTICK_H_ */