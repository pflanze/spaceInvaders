//init.h

#ifndef _INIT_H
#define _INIT_H
#include "debug.h"

//Timer2a
#define PERIOD 7273

// void init_Hw(void);  -- now in SpaceInvaders.c
#ifndef TEST_WITHOUT_IO
EXPORTED void Systick_Init(unsigned long periodSystick);
EXPORTED void Timer1A_Init(void);
EXPORTED void Timer2A_Init(void);
#endif
EXPORTED void Buttons_Init(void);
EXPORTED void ADC0_Init(void);
EXPORTED void DAC_Init(void);
#ifndef TEST_WITHOUT_IO
EXPORTED void LED_Init(void);
#endif

#if PORTF1_systick || PORTF1_audio
EXPORTED void PortF_init(void);
#endif

#endif
