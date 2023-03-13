void SystemInit() {}

#define SYSCTL_RCGC_ADC_R 				(*(( volatile unsigned long * ) 0x400FE638 ))
#define SYSCTL_RCGC_GPIO_R 				(*(( volatile unsigned long * ) 0x400FE608 ))

#define GPIO_PORTE_DEN_R 				  (*(( volatile unsigned long * ) 0x4002451C ))
#define GPIO_PORTE_AMSEL_R 				(*(( volatile unsigned long * ) 0x40024528 ))

#define ADC0_ACTIVE_SS_R				  (*(( volatile unsigned long * ) 0x40038000 ))
#define ADC0_INT_MASK_R					  (*(( volatile unsigned long * ) 0x40038008 ))
#define ADC0_TRIGGER_MUX_R				(*(( volatile unsigned long * ) 0x40038014 ))
#define ADC0_PROC_INIT_SS_R				(*(( volatile unsigned long * ) 0x40038028 ))
#define ADC0_PERI_CONFIG_R				(*(( volatile unsigned long * ) 0x40038FC4 ))
#define ADC0_INT_STATUS_CLR_R			(*(( volatile unsigned long * ) 0x4003800C ))
#define ADC0_SAMPLE_AVE_R				  (*(( volatile unsigned long * ) 0x40038030 ))
#define ADC0_SSPRI_R					    (*(( volatile unsigned long * ) 0x40038028 ))

#define ADC_SS3_IN_MUX_R 			   	(*(( volatile unsigned long * ) 0x400380A0 ))
#define ADC_SS3_CONTROL_R 				(*(( volatile unsigned long * ) 0x400380A4 ))
#define ADC_SS3_FIFO_DATA_R 			(*(( volatile unsigned long * ) 0x400380A8 ))

#define NVIC_EN0_R 						    (*(( volatile unsigned long * ) 0xE000E100 ))
#define NVIC_EN0_INT17 				     	0x00020000 // Interrupt 17 for ADC0 Seq0

#define GPIO_PORTF_DATA_R         (*((volatile unsigned long * ) 0x400253FC))
#define GPIO_PORTF_DIR_R          (*((volatile unsigned long * ) 0x40025400))
#define GPIO_PORTF_DEN_R          (*((volatile unsigned long * ) 0x4002551C))

#define GPIO_PORTD_DATA_R         (*((volatile unsigned long * ) 0x400073FC))
#define GPIO_PORTD_DIR_R          (*((volatile unsigned long * ) 0x40007400))
#define GPIO_PORTD_DEN_R          (*((volatile unsigned long * ) 0x4000751C))


void ADC_Init(void);
void ADC_Start_Sampling(void );
void delay(unsigned long value);


float VOLTAGE_VALUE = 0.0;

void ADC0Seq3_Handler(void) {
	unsigned int adc_data = 0;

	delay(10);

	adc_data = (ADC_SS3_FIFO_DATA_R & 0xFFF);
	VOLTAGE_VALUE = (((float) adc_data / 4095) * 3.3) - 0.03;

	ADC0_INT_STATUS_CLR_R |= 0x08;
}

void ADC_Init(void)
{		
	SYSCTL_RCGC_GPIO_R	|=	0x38;
	SYSCTL_RCGC_ADC_R 	|=	0x01;

	GPIO_PORTE_DEN_R 	&=	~(0x04);
	GPIO_PORTE_AMSEL_R 	|=	0x04;

	GPIO_PORTF_DIR_R |= 0x0F;
	GPIO_PORTF_DEN_R |= 0x0F; 

	GPIO_PORTD_DIR_R |= 0x0F;
	GPIO_PORTD_DEN_R |= 0x0F; 

	delay(3);

	ADC0_PERI_CONFIG_R 	&=	~0x00;			// Clear the sample rate
	ADC0_PERI_CONFIG_R 	|=	0x03;			// Set sample rate equal to 250 ksps
	ADC_SS3_IN_MUX_R 	=	0x01;			// Select AN1 (PE2) as Analog Input

	ADC0_SSPRI_R 		|=	0x0123;			// Sample sequence 3 has highest and SS0 has lowest priority
	ADC0_ACTIVE_SS_R 	&=	~(0x08);		// Disable sample sequence 3 before configuration

	ADC_SS3_CONTROL_R	|=	0x06;			// Enable TS0,IE0 and END0 bits

	ADC0_SAMPLE_AVE_R 	|=	0x04;			// Enable 16x hardware over sampling

	ADC0_INT_MASK_R 	|=	0x08;			// Unmask ADC0 Sequencer 3 Interrupt
	NVIC_EN0_R 			=	NVIC_EN0_INT17;	// Enable ADC0 Sequencer 3 Interrupt in NVIC
	ADC0_ACTIVE_SS_R 	|=	0x08;			// Activate Sample Sequencer 3
}

void ADC_Start_Sampling(void)
{
	ADC0_PROC_INIT_SS_R |= 0x08;
}

void delay(unsigned long value) {
	unsigned long i;
	for (i=0; i < value; i++) {}
}

int main() {
	ADC_Init();

	while (1) {
		ADC_Start_Sampling();
		delay(50000);
		if (VOLTAGE_VALUE > 1.5) {
			GPIO_PORTD_DATA_R |= 0x04;
			GPIO_PORTF_DATA_R = 0x08;
		} else {
			GPIO_PORTD_DATA_R &= 0x00;
			GPIO_PORTF_DATA_R = 0x02;
		}
	}
}
