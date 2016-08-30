/*! @file ADC.c
 *
 *  @brief Analog-to-Digital Converter module.
 *
 *  This contains the functions for converting analog to digital.
 *
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */
#include "LCD.h"
#include "ADC.h"

bool ADC_Init(void) {
  //Clear PORTA and Set to Input
  PORTA = 0;
  TRISAbits.TRISA0 = 1; //Input - Short Range IR
  TRISAbits.TRISA1 = 1; //Input - Long Range IR
  TRISAbits.TRISA3 = 0; //Output - +Vref (@4.09v)

  ADCON1bits.ADCS2 = 0; //Clock: Fosc/32
  ADCON0bits.ADCS1 = 1;
  ADCON0bits.ADCS0 = 0;
  ADCON0bits.CHS2 = 0;  //We are using channel 0 (AN0) for IR sensor
  ADCON0bits.CHS1 = 0;
  ADCON0bits.CHS0 = 0;

  ADCON1bits.ADFM = 1;  //Right justify the ADRESH:ADRESL
  ADCON1bits.PCFG3 = 0; //PORTE Digital, PORTA Analog
  ADCON1bits.PCFG2 = 0;
  ADCON1bits.PCFG1 = 1;
  ADCON1bits.PCFG0 = 0;

  ADCON0bits.ADON = 1; //Power up the A/D converter

  __delay_us(50); // Delay for ADC aquisition

  return true;
}

unsigned int ADC_GetVal(void) {
  volatile unsigned int adcRAW;
  ADRESH = 0;          // Reset the ADRES value registers
  ADRESL = 0;
  
  GO = 1;              // Start the ADC process
  while (GO) continue; // Wait for conversion complete

  /* Get the 8-bits from the ADRES registers (right-justified)*/
  adcRAW = (ADRESL + (ADRESH * 256));
  return (adcRAW);
}