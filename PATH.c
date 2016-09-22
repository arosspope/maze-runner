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
#include "PATH.h"

#define F_MASK 0b10000000
#define R_MASK 0b01000000
#define B_MASK 0b00100000
#define L_MASK 0b00010000
#define V_MASK 0b00001000
#define H_MASK 0b00000100
#define C_MASK 0b00000010


static uint8_t RotationFactor; /*< Used to determine the maps direction in relation to the robot (1-4)*/
static uint8_t Map[5][4] = {
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

bool PATH_Init(void){
  RotationFactor = 0;
  return true;
}

uint8_t PATH_GetMapInfo(uint8_t x, uint8_t y, TBOX_INFO info){
  uint8_t temp, box = 0;
  
  if(x < 5 && y < 4)
  {
    temp = Map[x][y];
    
    switch(info)
    {
      case BOX_Front:
        box = (temp & F_MASK);
        break;
      case BOX_Right:
        box = (temp & R_MASK);
        break;
      case BOX_Back:
        box = (temp & B_MASK);
        break;
      case BOX_Left:
        box = (temp & L_MASK);
        break;
      case BOX_Vic:
        box = (temp & V_MASK);
        break;
      case BOX_Home:
        box = (temp & H_MASK);
        break;
      case BOX_BeenThere:
        box = (temp & C_MASK);
        break;
      case BOX_All:
        box = temp;
        break;
    }
  }
  
  return box;
}

uint8_t normaliseBoxVal(uint8_t x, uint8_t y){
  
}