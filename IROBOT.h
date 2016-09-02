/*! @file IROBOT.c
 *
 *  @brief Module to control the iRobot.
 *
 *  This contains the functions for communicating with the iRobot through the
 *  create open interface.
 *
 *  @author
 *  @date 02-09-2016
 */
#ifndef IROBOT_H
#define	IROBOT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

/*! @brief Sets up the iRobot before first use.
 *
 *  @return BOOL - true if the iRobot was successfully initialized.
 */
bool IROBOT_Init(void);

/*! @brief Performs a 360 scan of the environment using the IR sensor. At the
 *  completion of the scan, the sensor will pointed towards the closest object
 *
 *  @return void
 */
void IROBOT_Scan360(void);

#ifdef	__cplusplus
}
#endif

#endif	/* IROBOT_H */

