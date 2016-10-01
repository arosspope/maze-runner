/*! @file PATH.c
 *
 *  @brief Robot path-finding through the maze
 *
 *  This module contains the routines in moving the iRobot through the maze. It
 *  defines the maze. 
 *
 *  @author A.Pope, J.Lynch
 *  @date 22-09-2016
 */
#ifndef PATH_H
#define	PATH_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

typedef enum {
  BOX_Front,
  BOX_Right,
  BOX_Back,
  BOX_Left,
  BOX_PFront,
  BOX_PRight,
  BOX_PBack,
  BOX_PLeft,
  BOX_All
} TBOX_INFO; /*< User can specify which information about a BOX they want to grab */

typedef struct
{
  uint8_t x;
  uint8_t y;
} TORDINATE; /*< Specifies an x and y coordinate for a position on the grid */

extern uint8_t PATH_RotationFactor; /*< The rotation factor of the robot in relation to the map */
extern int8_t PATH_Path[5][4];   /*< Specifies the path between the robot and a waypoint */

/*! @brief Sets up the PATH module before first use.
 *
 *  @return bool - TRUE if PATH was successfully initialized.
 */    
bool PATH_Init(void);

/*! @brief Returns the information about a box within the maze.
 *
 *  @param boxOrd - The coordinates of the box to obtain information about
 *  @param info - User can specify what information they want returned.
 *
 *  @return 8-bit number - For all info specifiers (aparat from 'BOX_All') you
 *                         can treat this return value as a boolean.
 */
uint8_t PATH_GetMapInfo(TORDINATE boxOrd, TBOX_INFO info);

/*! @brief Updates the maps orientation in relation to the robot.
 *
 *  This function MUST be called every time the robot rotates in a particular direction.
 *
 *  @param num90Turns - Number of 90 degree turns the robot has moved.
 *  @param dir - The direction in which the robot rotated in
 *
 *  @return void
 */
void PATH_UpdateOrient(uint8_t num90Turns, TDIRECTION dir);

/*! @brief Calculates a path between the robot and a waypoint within the maze.
 *
 *  This function MUST be called every time the layout of the maze changes (e.g.
 *  a virtual wall is detected).
 *
 *  @param robotOrd - The current coordinates of the robot
 *  @param waypOrd  - The coordinates of the waypoint to get too.
 *
 *  @return TRUE - If successfull
 */
bool PATH_Plan(TORDINATE robotOrd, TORDINATE waypOrd);

/*! @brief Used to indicate that a virtual wall was found in the maze space. Will
 *         update Map accordingly.
 *
 *  NOTE: This function assumes the virtual wall was found in front of the robot
 *  in the maze.
 *
 *  @param ord - The coordinate where the virtual wall was found.
 */
void PATH_VirtWallFoundAt(TORDINATE ord);
#ifdef	__cplusplus
}
#endif

#endif	/* PATH_H */

