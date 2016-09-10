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

/* Transmit and Recieve FIFOs */
static TFIFO TxFIFO, RxFIFO;

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

  TRISCbits.TRISC6 = 1; //Input: USART_RX - TODO: Must Confirm correct?
  TRISCbits.TRISC7 = 0; //Output USART_TX

  //Interrupts
  PIE1bits.RCIE = 1;  //Receive interrupt Enabled
  PIE1bits.TXIE = 0;  //Transmit interrupt disabled

  //Initialise the FIFOs
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);
  
  return true;
}

uint8_t USART_InChar(void)
{
  uint8_t data;
  bool gotData = false;
  //TODO: Because FIFO_get uses critical sections, the isr may not get a chance to put
  //data in the RxFIFO. Will need to test and investigate.

  //Wait until data has been received
  while(!gotData){
    di(); //Enter Critical Section
    gotData = FIFO_Get(&RxFIFO, &data); //Attempt to get data out of receive fifo
    ei(); //Exit Critical Section

    //Check if OERR has been set, if so - must clear in software (p.117 of PIC datasheet)
    if(RCSTAbits.OERR){
      RCSTAbits.CREN = 0;
      RCSTAbits.CREN = 1;
    } //TODO: This might be better placed in ISR?
  }

  return data;
}
 
void USART_OutChar(const uint8_t data)
{
  bool spaceInFIFO = false;
  
  //Wait until space in the TxFIFO becomes available, then put data into fifo
  while(!spaceInFIFO){
    di(); //Enter Critical Section
    spaceInFIFO = FIFO_Put(&TxFIFO, data);
    ei(); //Exit Critical Section
  }

  PIE1bits.TXIE = 1; //Arm transmission interrupt
}

void USART_ISR(void)
{
  //If recieve flag has been triggered, put data into the receive FIFO
  if(PIR1bits.RCIF){
    FIFO_Put(&RxFIFO, RCREG);
  }
  
  //If transmitt flag is okay, and we have actually enabled transmitt interrupts
  if(PIR1bits.TXIF && PIE1bits.TXIE){
    //Put data from the TxFIFO into TXREG
    if (!FIFO_Get(&TxFIFO, (uint8_t *)&TXREG)){
      PIE1bits.TXIE = 0; //Disable transmit interrupt if no bytes left in FIFO
    }
  }
}