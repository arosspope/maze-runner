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
#include "FIFO.h"
#define BAUD_57600 20

static TFIFO TxFIFO; //Transmitter FIFO buffer
static TFIFO RxFIFO; //Receiver FIFO buffer

bool USART_Init(void)
{
  //TXSTA
  TXSTAbits.TX9  = 0; //8 bit transmission
  TXSTAbits.TXEN = 1; //Transmit enable
  TXSTAbits.SYNC = 0; //Async mode
  TXSTAbits.BRGH = 1; //High speed baud rate for async mode

  //RCSTA
  RCSTAbits.SPEN  = 1; //Serial port enabled
  RCSTAbits.RX9   = 0; //8 bit recieve
  RCSTAbits.CREN  = 1; //Enable continous receive
  RCSTAbits.ADDEN = 0; //Disable address detection
  
  //Set the Baud Rate
  SPBRG = BAUD_57600;
  
  //Interrupts
  PIE1bits.RCIE = 0;  //Receive interrupt disabled
  PIE1bits.TXIE = 0;  //Transmit interrupt disabled
  
  //Init the FIFOs
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);
  
  return true;
}

bool USART_InChar(uint8_t * const dataPtr)
{
  return (FIFO_Get(&RxFIFO, dataPtr));  //Attempt to get data from the receive fifo
}
 
bool USART_OutChar(const uint8_t data)
{
  return (FIFO_Put(&TxFIFO, data));     //Attempt to put data into the transmit fifo
}

void USART_Poll(void)
{
  uint8_t tempData;
  
  //Check if data available
  if (PIR1bits.RCIF)
  {
    FIFO_Put(&RxFIFO, RCREG); //Get data from RCREG, and put into the receive FIFO
  }
  
  //Check if RCREG is ready for data
  if (TXSTAbits.TRMT)
  {
    if (FIFO_Get(&TxFIFO, &tempData)) //Attempt to get data from the transmit fifo
    {
      RCREG = tempData;
    }
  }
}