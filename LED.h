/*! @file LED.h
 *
 *  @brief LED routines for the PIC16F87XA.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */

#ifndef LED_H
#define	LED_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

/* Mapping LEDs within the system to specific PIC PORT pins */
#define LED_0 RB0
#define LED_1 RB1

/*! @brief Sets up the LEDs before first use.
 *
 *  @return bool - TRUE if the LEDs were successfully initialized.
 */
bool LED_Init(void);

#ifdef	__cplusplus
}
#endif
#endif	/* LED_H */

