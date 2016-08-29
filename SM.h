/*! @file SM.h
 *
 *  @brief Stepper Motor routines.
 *
 *  This contains the functions for operating the Stepper Motor.
 *
 *  @author Andrew Pope (sn: 11655949)
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
  DIR_CW,
  DIR_CCW
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
 *  @note Assumes that SM_Init has been called.
 *  @note Direction is dependent upon the current state of the system
 */
void SM_Move(unsigned int steps, TDIRECTION dir);

#ifdef	__cplusplus
}
#endif

#endif	/* SM_H */
