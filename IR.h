/*! @file IR.c
 *
 *  @brief Routines for the IR sensor.
 *
 *  This contains the functions for operating Infra-Red (IR) distance sensor.
 *
 *  @author Andrew.P, Andrew.T
 *  @date 02-08-2016
 */
#ifndef IR_H
#define	IR_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "types.h"

/*! @brief Sets up the IR sensor peripheral before first use.
 *
 *  @return bool - TRUE if the IR sensor was successfully initialized.
 */
bool IR_Init(void);

/*! @brief The IR sensor will perform a distance measurement
 *
 *  @return distance - Returns the measured distance in mm
 *  @note Assumes that LCD_Init has been called.
 */
double IR_Measure(void);
#ifdef	__cplusplus
}
#endif

#endif	/* IR_H */

