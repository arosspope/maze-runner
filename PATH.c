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
#include "LCD.h"

#define WALL_MASK 0b11110000
#define INFO_MASK 0b00001111
#define F_MASK 0b10000000
#define R_MASK 0b01000000
#define B_MASK 0b00100000
#define L_MASK 0b00010000
#define V_MASK 0b00001000
#define H_MASK 0b00000100
#define C_MASK 0b00000010

#define P_MAPPED 0b10000000
#define P_VAL    0b00001111
/* Private Function prototypes */
uint8_t getNormaliseBoxVal(uint8_t x, uint8_t y);
/* End prototypes */

uint8_t PATH_RotationFactor;
static uint8_t Map[5][4] = {  /*< Digital map of the maze space */
  {0b10110000, 0b11000000, 0b10010000, 0b11000000},
  {0b10010000, 0b01100000, 0b01010000, 0b01110100},
  {0b00010000, 0b10100000, 0b00000000, 0b11100000},
  {0b00010000, 0b11100000, 0b01010000, 0b11010000},
  {0b00110000, 0b10100000, 0b00100000, 0b01100000}
};

static uint8_t Path[5][4] = {  /*< Digital map of the maze space */
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  {0, 0, 0, 0}
};

bool PATH_Init(void){
  PATH_RotationFactor = 0;
  return true;
}

TORDINATE getNextBoxAt(TORDINATE ord, TBOX_INFO info)

bool PATH_Plan(TORDINATE ord){
  uint8_t i, j, k;
  uint8_t boxCount = 0;
  
  //Reset the Path map
  for(i = 0; i < 5; i++){
    for(j = 0; j < 4; j++){
      Path[i][j] = 0;
    }
  }
  
  
  while(boxCount != 20){
    if(!PATH_GetMapInfo(ord.x, ord.y, BOX_Front)){

    }
    
    if(!PATH_GetMapInfo(ord.x, ord.y, BOX_Left)){
      
    }
    
    if(!PATH_GetMapInfo(ord.x, ord.y, BOX_Right)){
      
    }
    
    if(!PATH_GetMapInfo(ord.x, ord.y, BOX_Back)){
      
    }
  }
  
  return true;
}

uint8_t PATH_GetMapInfo(uint8_t x, uint8_t y, TBOX_INFO info){
  uint8_t temp, box = 0;
  
  if(x < 5 && y < 4)
  {
    temp = getNormaliseBoxVal(x, y);

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

void PATH_UpdateOrient(uint8_t num90Turns, TDIRECTION dir){
  int8_t temp;

  if(dir == DIR_CW){
    PATH_RotationFactor = (PATH_RotationFactor + num90Turns) % 4; //CW direction is 'positive movement'
  } else {
    //If CCW direction, we must make sure we are moduloing within 4 states (0,1,2,3)
    //therefore, we must check for negative values.
    temp = PATH_RotationFactor - num90Turns;
    temp = temp % 4;

    if (temp < 0)
    {
      temp += 4;
    }
    
    PATH_RotationFactor = temp;
  }
}

/*! @brief Normalises the wall location in relation to the robot for a
 *  particular square.
 *
 *  @param x - horizontal co-ordinate
 *  @param y - vertical co-ordinate
 *
 *  @return 8-bit number - Normalised info about the box.
 */
uint8_t getNormaliseBoxVal(uint8_t x, uint8_t y){
  /* We only want to normalise the values in the map boxes that correspond to the
   * 4 walls. To 'normalise' this value, we must bit shift the box values by the
   * amount of times the robot has rotated in the system.
   *
   * As long as the RotationFactor is either (0, 1, 2, 3) this algorithm will work.
   *
   * Example:
   *  - Consider we have box value = 1001 0000, with RotationFactor = 3
   *  - After normalisation, the box value = 1100 0000
   *
   * We will use this example box value and RotationFactor
   * to illustrate the algorithm.
   */
  uint8_t temp;
  uint8_t info = (Map[x][y] & INFO_MASK);
  uint8_t boxval = (Map[x][y] & WALL_MASK) >> 4; //Eg: bv = 0000 1001

  boxval = boxval << PATH_RotationFactor;    //Eg. bv = 0100 1000
  temp = boxval << 4;                   //Eg. temp = 1000 0000
  boxval = (temp | boxval) & WALL_MASK; //Eg. ((1000 0000) | (0100 1000)) & WALL_MASK) 
                                        //    = (1100 0000) Normalised box value!
  return (boxval | info);
}
