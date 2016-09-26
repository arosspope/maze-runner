/*! @file IROBOT.c
 *
 *  @brief Module to move the iRobot.
 *
 *  This contains the functions for moving the iRobot through the create open
 *  interface.
 *
 *  @author A.Pope, K.Leone, C.Stewart, J.Lynch, A.Truong
 *  @date 02-09-2016
 */
#ifndef MOVE_H
#define	MOVE_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "types.h"

typedef union
{
  uint8_t sensors;		/*!< The status bits accessed as a byte. */
  struct
  {
    uint8_t bump     : 1;	/*!< The bump status bit. */
    uint8_t wall     : 1;	/*!< The wall status bit. */
    uint8_t victim   : 1;	/*!< The victim status bit. */
  } sensBits;
} SensorsStatus_t;   

/*! @brief Sets up the move module before first use
 *
 *  @return bool - TRUE if the move module was successfully initialized.
 */
bool MOVE_Init(void);

/* @brief Rotates the robot to a particular orientation (angle within a circle).
 *
 * @param velocity - The velocity to turn at (positive value only).
 * @param angle - Angle to rotate through in specified direction.
 * @param dir - The direction to rotate
 *
 * @return bool - True if movement was interrupted by sensor
 */
bool MOVE_Rotate(uint16_t velocity, uint16_t angle, TDIRECTION dir);

/*! @brief Drive the robot in a straight line.
 *
 *  @param velocity - Speed at which the robot can move (-500 - 500 mm/s)
 *  @param distance - distance that the robot must travel.
 *  @return bool - True if movement was interrupted by a sensor
 */
bool MOVE_Straight(int16_t velocity, uint16_t distance);

/* @brief Will tell the iRobot to start moving each will at particular velocity.
 *        No distance checking is used. Robot needs to be explicity stopped.
 *
 * @param leftWheelVel - The velocity of the left side wheel
 * @param rightWheelVel - The velocity of the right side wheel
 */
void MOVE_DirectDrive(int16_t leftWheelVel, int16_t rightWheelVel);

/* @brief Determines if any of the relevant sensors have been triggered.
 *
 * @return TRUE - if one of the sensors have been tripped
 */
bool MOVE_CheckSensor(SensorsStatus_t * sensStatus);
#ifdef	__cplusplus
}
#endif

#endif	/* MOVE_H */