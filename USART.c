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

typedef enum {
  USART_TX,
  USART_RX
} USART_MODE;

void changeMode(USART_MODE mode);

bool USART_Init(void)
{
  //TXSTA
  TXSTAbits.TX9  = 0; //8 bit transmission
  TXSTAbits.TXEN = 1; //Transmit enable
  TXSTAbits.SYNC = 0; //Async mode
  TXSTAbits.BRGH = 1; //High speed baud rate for async mode

  //RCSTA
  RCSTAbits.SPEN  = 1; //Serial port enabled
  RCSTAbits.RX9   = 0; //8 bit receive
  RCSTAbits.CREN  = 1; //Enable continous receive
  RCSTAbits.ADDEN = 0; //Disable address detection
  
  //Set the Baud Rate
  SPBRG = BAUD_57600;
  
  //Interrupts
  PIE1bits.RCIE = 0;  //Receive interrupt disabled
  PIE1bits.TXIE = 0;  //Transmit interrupt disabled
  
  return true;
}

uint8_t USART_InChar(void)
{
  //TODO: Slightly inefficent to sit here polling - may need to use interrupts
  uint8_t data;
  changeMode(USART_RX);
  while(!(PIR1bits.RCIF));  //Wait until data is availalbe
  data = RCREG;       //Read the register

  return data;
}
 
void USART_OutChar(const uint8_t data)
{
  //TODO: Slightly inefficent to sit here polling - may need to use interrupts
  changeMode(USART_TX);
  while(!TXSTAbits.TRMT); //Wait until the buffer becmoes empty
  TXREG = data;           //Load register with data to transmit
}

void changeMode(USART_MODE mode){
  /*** Note this is somewhat of a 'hacky' solution ***/
  /*** TODO: Ask kyle why it is necessary to switch data direction on PORTC ***/
  if(mode == USART_RX){
    TRISCbits.TRISC6 = 1; //Setup PORTC pins for USART Receive 
    TRISCbits.TRISC7 = 0; 
  }else{
    TRISCbits.TRISC6 = 1; //Setup PORTC pins for USART Transmit
    TRISCbits.TRISC7 = 1; 
  }
}