//LED.h
#ifndef _LED_H
#define _LED_H


void Red(void);

void Green(void);

#ifdef TEST_WITHOUT_IO
extern unsigned char GPIO_PORTB_DATA_R;
#endif

#endif
