#include "encoder.h"
#include "common_defs.h"

ISR(PCINT_Vect_Part);

// Inicializace Encoderu (pinu) a nastaven� Pull-Up rezistor�
void ENC_InitEncoder(void)
{
	// Nastaven� Encoderu
	// Vstupn� A0 a A1
	sbi(ENC_WR, ENC_A);
	sbi(ENC_WR, ENC_B);
	// Pull-Up rezistory
	sbi(ENC_CTL, ENC_A);
	sbi(ENC_CTL, ENC_B);
}

// Nasteven� p�eru�en� pro Encoder
void ENC_Intterupt_Set(void)
{
	// Nastaven� p�eru�en� pro Encoder
	PCMSK_Registr |= (1 << PCINT_ENC_A) | (1 << PCINT_ENC_B);  //enable encoder pins interrupt sources
	
	/* enable pin change interupts */
	PCICR |= (1 << PCIE0_Part );
}

