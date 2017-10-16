//LED.c

static unsigned char n;

void Red(void){
	for(i=0;n<4;n++){
	 GPIO_PORTB_DATA_R ^= 0x10;
	}
}


void Green(void){
	for(i=0;n<4;n++){
	 GPIO_PORTB_DATA_R ^= 0x20;
	}
}