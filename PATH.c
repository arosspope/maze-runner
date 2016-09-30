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
#define F_MASK 0b10000000 //Front wall of the box
#define R_MASK 0b01000000 //Right-hand wall of box
#define B_MASK 0b00100000 //Back/Behind wall of box
#define L_MASK 0b00010000 //Left-hand wall of box
#define V_MASK 0b00001000 //Victim within box?
#define H_MASK 0b00000100 //Home base?
#define C_MASK 0b00000010 

/* Private Function prototypes */
uint8_t getNormalisedBoxVal(uint8_t x, uint8_t y);
int8_t highestNeighbourCell(uint8_t x, uint8_t y);
/* End prototypes */

uint8_t PATH_RotationFactor;
static uint8_t Map[5][4] = {  /*< Digital map of the maze space */
  {0b10110000, 0b11000000, 0b10010000, 0b11000000},
  {0b10010000, 0b01100000, 0b01010000, 0b01110100},
  {0b00010000, 0b10100000, 0b00000000, 0b11100000},
  {0b00010000, 0b11100000, 0b01010000, 0b11010000},
  {0b00110000, 0b10100000, 0b00100000, 0b01100000}
};

int8_t PATH_Path[5][4]; /*< Path using flood fill method*/

bool PATH_Init(void){
  PATH_RotationFactor = 0;
  return true;
}

bool PATH_Plan(TORDINATE robotOrd, TORDINATE waypOrd){
  bool done = false;
  int8_t currentPathDistance; //How far the 'water' has flowed
  uint8_t x, y;
  
  //Reset the Path map - Each value contains '-1' to signify no path
  for(x = 0; x < 5; x++){
    for(y = 0; y < 4; y++){
      PATH_Path[x][y] = -1;
    }
  }

  PATH_Path[waypOrd.x][waypOrd.y] = 0; //Set the waypoint to flood point 0
  
  while(!done) //Begin flooding the maze until we find a path
  {
    for(x = 0; x < 5; x++){
      for(y = 0; y < 4; y++)
      {
        if(PATH_Path[x][y] != -1)  //If the cell has already been reached, then skip this cell
          continue;

        //If there is a neighbour cell that has been reached, then the current cell is next
        //in line for the fill
        currentPathDistance = highestNeighbourCell(x,y);
        if(currentPathDistance != -1)
          PATH_Path[x][y] = (currentPathDistance+1);
      }
    }

    if(PATH_Path[robotOrd.x][robotOrd.y] != -1) //A Path has been found to the robot
      done = true;

    //currentPathDistance++; //Raise the flood 'waters'
  }
  LCD_PrintInt(currentPathDistance, TOP_LEFT);
  return true;
}

uint8_t PATH_GetMapInfo(TORDINATE boxOrd, TBOX_INFO info){
  uint8_t temp, box = 0;
  
  if(boxOrd.x < 5 && boxOrd.y < 4)
  {
    temp = getNormalisedBoxVal(boxOrd.x, boxOrd.y);

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
uint8_t getNormalisedBoxVal(uint8_t x, uint8_t y){
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

/*! @brief Finds and returns the highest 'flood' number in an accessible
 *         neighbouring cell.
 *
 *  @param x - horizontal co-ordinate of current pos
 *  @param y - vertical co-ordinate of current pos
 *
 *  @return 8-bit number - The highest 'flood' number found
 */
int8_t highestNeighbourCell(uint8_t x, uint8_t y){
  int8_t highestVal = -1;
  int8_t valAtNext;

  if(!(Map[x][y] & F_MASK)){ //If there is not a wall in front of us on map
    valAtNext = PATH_Path[(x-1)][y];
    //If the value of that next box to the front of us has a value thats
    //greater than the biggest seen so far, then set it.
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  if(!(Map[x][y] & L_MASK)){
    valAtNext = PATH_Path[x][(y-1)];
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  if(!(Map[x][y] & R_MASK)){
    valAtNext = PATH_Path[x][(y+1)];
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  if(!(Map[x][y] & B_MASK)){
    valAtNext = PATH_Path[(x+1)][y];
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  return highestVal;
}
