/*! @file SM.h
 *
 *  @brief Stepper Motor routines.
 *
 *  This contains the functions for operating the Stepper Motor.
 *
 *  @author Andrew. P
 *  @date 02-08-2016
 */
#ifndef SM_H
#define	SM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"
    
typedef enum
{
  DIR_CW = 2,
  DIR_CCW = 0
} TDIRECTION; /* Used to define desired rotation for SM */
    
/*! @brief Sets up the stepper motor before first use
 *
 *  @return bool - TRUE if the Stepper Motor was successfully initialized.
 */
bool SM_Init(void);

/*! @brief Rotates the stepper motor in the desired amount of steps in certain direction.
 *
 *  @param steps - Number of half-steps to move
 *  @param dir - The direction to move in (CW || CCW)
 *  @return STEP_COUNT - returns the current step count of the system
 * 
 *  @note Assumes that SM_Init has been called.
 */
int SM_Move(unsigned int steps, TDIRECTION dir);

#ifdef	__cplusplus
}
#endif

#endif	/* SM_H */

