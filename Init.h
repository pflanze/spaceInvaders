//initialize HW

void init_Hw(void);							//Initializes the system (help to keep main clean)
void Systick_Init(unsigned long periodSystick);
void Timer2_Init(unsigned long period);
void PortF_init(void);
void Buttons_Init(void);
void ADC0_Init(void);
void DAC_Init(void);
void LED_Init(void);

