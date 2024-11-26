/********************************************************************************************************************************/
/**************************************** ANGEL HABID NAVARRO MENDEZ ************************************************************/
/****************************************      20110320 7K IDESI     ************************************************************/
/****************************************           DIMER            ************************************************************/
/********************************************************************************************************************************/

//Connections:
//Cambiar el ADC de canal para colocar con potenciometro externo ( AD4 CH4 PTB0).
// Canal zero para input capture (TPM CH0 PTA0).
//Canal 1 para PWM (TPM CH 1, PTB5).

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */


void MCU_init (void);
void output_control(void);
void init_inputcapture(void);
void ADC_init(void);
unsigned int adc_leer(unsigned char t);
void PWM(unsigned int periodo, unsigned int ancho);
void IRQ_cross2zero(void);

unsigned char canal = 0x04;
unsigned int ADCresult=0X0000;
unsigned int ADCresult1=0x0000;
unsigned int flag_zero = 0;

void main(void) {
	
	MCU_init(); 
  
  for(;;) {
	EnableInterrupts;
	if(flag_zero == 1){
	
		DisableInterrupts;
		  output_control();
		//flag_zero = 0;
	 }  
  }
}

void MCU_init(void){
	/*Kill the Dog*/
	  SOPT1 = 0x12;
	/*Basic initialisations */  
	  ADC_init();
	  init_inputcapture();
}

void output_control(void){//esta funcion contiene el main del codigo para PWM y ADC
	
	ADCresult = adc_leer(canal)*8;
	
	if(ADCresult1<ADCresult-30 ||ADCresult1>ADCresult+30){
		ADCresult1=ADCresult;
		TPMCNT = 0X0000;
		TPMSC=0X00;
	}
	PWM(2083,ADCresult1);//2083 CUENTAS = 16.666ms = 60hz
		
}

void PWM(unsigned int periodo, unsigned int ancho)
{
TPMMOD=periodo;//time of the cycle (valued in quantity of counts). 
TPMC1V=ancho + 20;//High time + offset.
TPMSC = 0x0D; // Sin IRQ, BusClk=4Mhz, Preescaler=32
TPMC1SC = 0x28; //ModoPWM, Aligned rising edge.
while(!TPMC1SC_CH1F);//Wait for the flag (End of High time).
TPMC1SC_CH1F=0; // Clear the flag to CH1 (TPM enabled as PWM).

}

unsigned int adc_leer(unsigned char t) {
    ADCSC1 = 0x04 | t;// Reset ADC CH 4.
    while (ADCSC1_COCO != 1);//wait for the conversion complete
    return ADCR;//return te result register.
}

void init_inputcapture(void)
{
	TPMSC = 0x0D; // Sin IRQ, BusClk=4Mhz, Preescaler=32
	//TPMMOD = 0xFFFF; // Maxima cuenta del Contador General del TPM //2083 CUENTAS = 16.666ms = 60hz
	//TPMC0V = 0x1000;  // Valor de Comparacion 
	//Se escribe al ultimo para reset del counter.
	TPMC0SC = 0x44; // 0100 0100 => Modo=Input capture, Deteccion = transicion de subida // FALLING EDGE.
}

void ADC_init(void){
	/* ADC initialisation*/
	ADCCFG = 0x10;//8bits/Bus clock/Log sample time/input clock/high speed.
	APCTL1_ADPC0 = 0x10;//Enable ADC CH4
	ADCSC1 = 0x04;//ADC CH 4, PTB0;
	ADCSC2 = 0x00;//Disable comparator
}

interrupt 5  void IRQ_cross2zero (void)
{
	DisableInterrupts;
	flag_zero = 1;
	TPMC0SC_CH0F=0; // Clear the flag to CH0 (TPM enabled as InputC).
	DisableInterrupts;
}


