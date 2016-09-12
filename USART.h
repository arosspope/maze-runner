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
 
/*! @brief Get a character from the RCREG.
 *
 *  @return data - Byte from the RCREG
 */
uint8_t USART_InChar(void);
 
/*! @brief Attempt to transmit a character through TXREG.
 *
 *  @param data The byte to be transmitted.
 *  @return bool - TRUE if if successfull.
 */
void USART_OutChar(const uint8_t data);

#ifdef	__cplusplus
}
#endif

#endif	/* USART_H */