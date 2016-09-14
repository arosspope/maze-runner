/*! @file ADC.h
 *
 *  @brief Analog-to-Digital Converter module.
 *
 *  This contains the functions for converting analog to digital.
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "types.h"

/*! @brief Sets up the ADC before first use.
 *
 *  @return bool - TRUE if the LCD were successfully initialized.
 */
bool ADC_Init(void);

/*! @brief Returns a digital value (converted from analog)
 *
 *  @return value - The converted digital value
 */
unsigned int ADC_GetVal(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

