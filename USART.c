/*! @file USART.c
 *
 *  @brief Serial Communication via USART.
 *
 *  This contains the functions for serial communication via the Universal
 *  Synchronous Asynchronous Receiver Transmitter.
 *
 *  @author Andrew.P 
 *  @date 02-08-2016
 */
#include "USART.h"
#define BAUD_57600 20

bool USART_Init(void)
{
  //Setup TRISC Register
  TRISCbits.TRISC6 = 1; //USART_TX
  TRISCbits.TRISC7 = 1; //USART_RX
  
  TXSTAbits.BRGH = 1; //High speed baud rate for async mode
  TXSTAbits.SYNC = 0; //Async mode
  RCSTAbits.SPEN = 1; //Serial port enabled
  RCSTAbits.CREN = 1; //Enable continous receive
  RCSTAbits.SREN = 0; //No effect
	
  PIE1bits.TXIE = 0; //Disable Interrupts
  PIE1bits.RCIE = 0;
  
  TXSTAbits.TX9 = 0; //8 bit transmission
  RCSTAbits.RX9 = 0; //8 bit receive
  
	//Reset the transmitter
  TXSTAbits.TXEN = 0;
  TXSTAbits.TXEN = 1; 
  
	SPBRG = BAUD_57600; //Baud rate 57600
  
  return true;
}

uint8_t USART_InChar(void)
{
  uint8_t data;
  
  while(!(PIR1bits.RCIF));  //Wait for data to become available
	data = RCREG;
  
  //If error during transmission, make sure to clear in software
  if(RCSTAbits.OERR){
    CREN = 0;
    CREN = 1;
  }
  
  return data;
}

void USART_OutChar(const uint8_t data)
{
  while(!(TXSTAbits.TRMT)); //While buffer is not empty, wait
	TXREG=data;               //load register with data to be transmitted
}