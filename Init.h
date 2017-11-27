//init.h

#ifndef _INIT_H
#define _INIT_H
#include "debug.h"

//Timer2a
#define PERIOD 7273

void init_Hw(void);							//Initializes the system (help to keep main clean)
void Systick_Init(unsigned long periodSystick);
void Timer1A_Init(void);
void Timer2A_Init(void);
void Buttons_Init(void);
void ADC0_Init(void);
void DAC_Init(void);
void LED_Init(void);

#if PORTF1_systick || PORTF1_audio
void PortF_init(void);
#endif

#endif
