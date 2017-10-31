//initialize HW

#ifndef _INIT_H
#define _INIT_H

//testing code
#define AUDIO	1
#define PORTF1_audio		1		//Enables/disables portF, Logic Analizer frequency test
#define PORTF1_systick	1		//Enables/disables portF, Logic Analizer frequency test

void init_Hw(void);							//Initializes the system (help to keep main clean)
void Systick_Init(unsigned long periodSystick);
void Timer2_Init(unsigned long period);
void Buttons_Init(void);
void ADC0_Init(void);
void DAC_Init(void);
void LED_Init(void);

#if PORTF1_systick || PORTF1_audio
void PortF_init(void);
#endif

#endif
