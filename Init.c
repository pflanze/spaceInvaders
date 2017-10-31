//Initialize HW

#ifndef TEST_WITHOUT_IO
#  include "..//tm4c123gh6pm.h"
#endif
#include "Init.h"


void Systick_Init(unsigned long periodSystick){
#ifndef TEST_WITHOUT_IO
  NVIC_ST_CTRL_R = 0;         						// disable SysTick during setup
  NVIC_ST_RELOAD_R = periodSystick;						// reload value
  NVIC_ST_CURRENT_R = 0;      						// any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; // priority 1
  NVIC_ST_CTRL_R = 0x0007; 								// enable,core clock, and interrupts
#endif
}


// You can use this timer only if you learn how it works
#if AUDIO
void Timer2_Init(void){ 
#ifndef TEST_WITHOUT_IO
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  delay = SYSCTL_RCGCTIMER_R;
  TIMER2_CTL_R &= ~0x00000001;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = PERIOD-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt (interrupt enable)
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
#endif
}
#endif

void Buttons_Init(){
#ifndef TEST_WITHOUT_IO
	//Clock for Port E already active
	GPIO_PORTE_AMSEL_R &= ~0x03; // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO on PE1-0
  GPIO_PORTE_DIR_R &= ~0x03;   // 5) inputs on PE1-0
  GPIO_PORTE_AFSEL_R &= ~0x03; // 6) regular function on PE1-0
	GPIO_PORTE_DEN_R |= 0x03; // 7) enable digital on PE1-0
#endif
}

void ADC0_Init(void){ 
#ifndef TEST_WITHOUT_IO
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;   // 1) activate clock for Port E
	delay = SYSCTL_RCGC2_R;         //    allow time for clock to stabilize
	GPIO_PORTE_DIR_R &= ~0x04;      // 2) make PE2 input
	GPIO_PORTE_AFSEL_R |= 0x04;     // 3) enable alternate function on PE2
	GPIO_PORTE_DEN_R &= ~0x04;      // 4) disable digital I/O on PE2
	GPIO_PORTE_AMSEL_R |= 0x04;     // 5) enable analog function on PE2
	
	SYSCTL_RCGC0_R |= 0x00010000;   // 6) activate ADC0 
	delay = SYSCTL_RCGC2_R;         
	SYSCTL_RCGC0_R &= ~0x00000300;  // 7) configure for 125K 
	ADC0_SSPRI_R = 0x0123;          // 8) Sequencer 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+1; // 11) channel Ain1 (PE2)
	ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
	ADC0_ACTSS_R |= 0x0008;         // 13) enable sample sequencer 3
#endif
}

//DAC PB3-0
void DAC_Init(void){
#ifndef TEST_WITHOUT_IO
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
	delay = SYSCTL_RCGC2_R;    // allow time to finish activating*/
	GPIO_PORTB_AMSEL_R &= ~0x0F;      // no analog function for PB3-0
	GPIO_PORTB_PCTL_R &= ~0x0000FFFF; // regular function for PB3-0
	GPIO_PORTB_DIR_R |= 0x0F;      // make PB3-0 out
	GPIO_PORTB_DR8R_R |= 0x0F;    // can drive up to 8mA out
	GPIO_PORTB_AFSEL_R &= ~0x0F;   // disable alt funct on PB3-0
	GPIO_PORTB_DEN_R |= 0x0F;      // enable digital I/O on PB3-0
#endif
}


// LED on PB4, PB5
//PB4 RED
//PB5 Green
void LED_Init(void){
#ifndef TEST_WITHOUT_IO
	//port B already active
	GPIO_PORTB_AMSEL_R &= ~0x30;      // no analog function for PB3-0
	GPIO_PORTB_PCTL_R &= ~0x0000FFFF; // regular function for PB3-0
	GPIO_PORTB_DIR_R |= 0x30;      // make PB3-0 out
	GPIO_PORTB_DR8R_R |= 0x30;    // can drive up to 8mA out
	GPIO_PORTB_AFSEL_R &= ~0x30;   // disable alt funct on PB3-0
	GPIO_PORTB_DEN_R |= 0x30;      // enable digital I/O on PB3-0
	
	GPIO_PORTB_DATA_R &= ~0x30;		//LEDs off by default
#endif
}


//initialize PortF !!!!testing Only!!!!
#if PORTF1_systick || PORTF1_audio
void PortF_init(void){ volatile unsigned long delay;
#ifndef TEST_WITHOUT_IO
	SYSCTL_RCGC2_R |= 0x00000020;     // activate clock for Port F
	delay = SYSCTL_RCGC2_R;           // allow time for clock to start
	GPIO_PORTF_DIR_R |= 0x02;      // 2) make PF1 output
	GPIO_PORTF_AFSEL_R &= ~0x02;     // 3) disable alternate function on PF1
	GPIO_PORTF_DEN_R |= 0x02;      // 4) enable digital I/O on PF1
	GPIO_PORTF_AMSEL_R &= ~0x02;     // 5) disable analog function on PF1
	GPIO_PORTF_DATA_R &= ~0x02;			//off by default
#endif
}
#endif
