/*! @file USART.h
 *
 *  @brief Serial Communication via USART.
 *
 *  This contains the functions for serial communication via the Universal
 *  Synchronous Asynchronous Receiver Transmitter.
 *
 *  @author Andrew.P 
 *  @date 02-08-2016
 */
#ifndef USART_H
#define	USART_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

/*! @brief Sets up the USART interface before first use.
 *
 *  @return BOOL - true if the USART was successfully initialized.
 */
bool USART_Init(void);
 
/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return bool - TRUE if the receive FIFO returned a character.
 */
bool USART_InChar(uint8_t * const dataPtr);
 
/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return bool - TRUE if the data was placed in the transmit FIFO.
 */
bool USART_OutChar(const uint8_t data);

/*! @brief Poll the USART status registers to try and receive and/or transmit one character.
 *
 *  @return void
 */
void UART_Poll(void);

#ifdef	__cplusplus
}
#endif

#endif	/* USART_H */