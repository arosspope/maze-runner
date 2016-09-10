/*! @file SPI.c
 *
 *  @brief Serial Port Interface.
 *
 *  This module is a HAL (Hardware abstraction Layer) for the SPI on the PIC.
 *  It facilitates communication between hardware and software for other modules
 *  such as SM, IR, and EEPROM.
 *
 *  @author Andrew.P
 *  @date 02-08-2016
 */
#include "SPI.h"

bool SPI_Init(void) {
  //Setup SSPCON & SSPSTAT Registers
  SSPSTATbits.SMP = 0; //Input data sampled at middle of data output time
  SSPSTATbits.CKE = 1; //Transmit occurs on transition from active to idle clock state

  SSPCONbits.WCOL = 0;  //No Collision
  SSPCONbits.SSPOV = 0; //No Overflow
  SSPCONbits.SSPEN = 1; //Enables serial port and serial port pins
  SSPCONbits.CKP = 0;   //Idle state for clock is a low level
  //SPI Master mode, clock Fosc/4
  SSPCONbits.SSPM3 = 0;
  SSPCONbits.SSPM2 = 0;
  SSPCONbits.SSPM1 = 0;
  SSPCONbits.SSPM0 = 0;

  //Setup data direction for PORTC 
  TRISCbits.TRISC0 = 0; //Output: CPLD_CS(0)
  TRISCbits.TRISC1 = 0; //Output: CPLD_CS(0)
  TRISCbits.TRISC2 = 0; //Output: SM_STEP
  TRISCbits.TRISC3 = 0; //Output: SPI_CLK
  TRISCbits.TRISC4 = 1; //Input: SPI_SDI
  TRISCbits.TRISC5 = 0; //Output: SPI_SDO
  
  SPI_SelectMode(SPI_NONE); //Select no mode initially
  
  return true;
}

void SPI_SelectMode(TSPI_MODE mode){
  PORTC |= mode; //Set the mode by changing CPLD(0, 1)
}

uint8_t SPI_SendData(uint8_t txData){
  uint8_t rxData;

  SSPIF = 0;          //Clear the SSPIF flag to initiate data transmit
  SSPBUF = txData;    //Load data to the SSPBUF buffer
  while(SSPIF == 0);  //Wait until the transmit process has completed

  rxData = SSPBUF;    //Potentially receieve data from the buffer
  SSPIF = 0;

  return rxData;
}