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
  BOX_Vic,
  BOX_Home,
  BOX_BeenThere,
  BOX_All
} TBOX_INFO;    
    
/*! @brief Sets up the PATH module before first use.
 *
 *  @return bool - TRUE if PATH was successfully initialized.
 */    
bool PATH_Init(void);


#ifdef	__cplusplus
}
#endif

#endif	/* PATH_H */

