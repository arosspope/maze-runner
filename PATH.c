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

#define VWALLS  0b11110000
#define PWALLS  0b00001111
#define FRONT   0b10000000 //Used to determine prescense of Virtual & Physical Walls
#define RIGHT   0b01000000 
#define BACK    0b00100000 
#define LEFT    0b00010000
#define P_FRONT 0b00001000 //Used to determine prescense of Physical Walls only
#define P_RIGHT 0b00000100
#define P_BACK  0b00000010
#define P_LEFT  0b00000001

/* Private Function prototypes */
uint8_t getNormalisedBoxVal(uint8_t x, uint8_t y);
int8_t highestNeighbourCell(uint8_t x, uint8_t y);
/* End prototypes */

uint8_t PATH_RotationFactor;
static uint8_t Map[5][4] = {  /*< Digital map of the maze space */
  {0b10111011, 0b11001100, 0b10011001, 0b11001100},
  {0b10011001, 0b01100110, 0b01010101, 0b01110111},
  {0b00010001, 0b10101010, 0b00000000, 0b11101110},
  {0b00010001, 0b11101110, 0b01010101, 0b11011101},
  {0b00110011, 0b10101010, 0b00100010, 0b01100110}
};

int8_t PATH_Path[5][4]; /*< Path between two waypoints using flood fill method */

bool PATH_Init(void){
  PATH_RotationFactor = 0;
  return true;
}

bool PATH_Plan(TORDINATE robotOrd, TORDINATE waypOrd){
  bool done = false;
  uint16_t loopCount = 0;     //Used to limit how many loops the function will iterate through the map to find a path
  int8_t currentPathDistance; //How far the 'water' has flowed
  uint8_t x, y;
  
  //Reset the Path map - Each value contains '-1' to signify no path
  for(x = 0; x < 5; x++){
    for(y = 0; y < 4; y++){
      PATH_Path[x][y] = -1;
    }
  }

  PATH_Path[waypOrd.x][waypOrd.y] = 0; //Set the way-point to flood point 0
  
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
          PATH_Path[x][y] = (currentPathDistance + 1);
      }
    }
    loopCount++;
    
    if(PATH_Path[robotOrd.x][robotOrd.y] != -1) //A Path has been found to the robot
      done = true;
    
    //Algorithm is allowed to loop a max of (20x20 + 1) times before deciding that a path cant be found
    if(loopCount > 401) 
      done = true;
  }

  return !((loopCount > 401));
}

uint8_t PATH_GetMapInfo(TORDINATE boxOrd, TBOX_INFO info){
  uint8_t temp, box = 0;
  
  if(boxOrd.x < 5 && boxOrd.y < 4) //Sanity check that the ordinate is not outside the map
  {
    //Normalizes the map direction in relation to the robot
    temp = getNormalisedBoxVal(boxOrd.x, boxOrd.y);
    //Will return the info required by the user
    switch(info)
    {
      case BOX_Front:
        box = (temp & FRONT);   break;
      case BOX_Right:
        box = (temp & RIGHT);   break;
      case BOX_Back:
        box = (temp & BACK);    break;
      case BOX_Left:
        box = (temp & LEFT);    break;
      case BOX_PFront:
        box = (temp & P_FRONT); break;
      case BOX_PRight:
        box = (temp & P_RIGHT); break;
      case BOX_PBack:
        box = (temp & P_BACK);  break;
      case BOX_PLeft:
        box = (temp & P_LEFT);  break;
      case BOX_All:
        box = temp;             break;
    }
  }
  
  return box;
}

void PATH_VirtWallFoundAt(TORDINATE ord){
  //Assume wall was found in front of robot, shift by rotation factor and assign
  uint8_t virtwall = (0b10000000) >> PATH_RotationFactor;
  Map[ord.x][ord.y] |= virtwall;
  
  //Make sure that this shared wall is updated as a virtual wall in the next 'sqaure'
  switch(PATH_RotationFactor){
    case 0:
      ord.x = ord.x - 1; break;
    case 1:
      ord.y = ord.y + 1; break;
    case 2:
      ord.x = ord.x + 1; break;
    case 3:
      ord.y = ord.y - 1; break;
  }
  
  //The back wall in the next square in front of the robot is updated as a virtual wall
  virtwall = (0b00100000) >> PATH_RotationFactor;
  Map[ord.x][ord.y] |= (virtwall << 4);
}

void PATH_UpdateOrient(uint8_t num90Turns, TDIRECTION dir){
  int8_t temp;

  if(dir == DIR_CW){
    PATH_RotationFactor = (PATH_RotationFactor + num90Turns) % 4; //CW direction is 'positive movement'
  } else {
    //If CCW direction, we must make sure we are modulo-ing within 4 states (0,1,2,3)
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

/*! @brief Normalizes the wall location in relation to the robot for a
 *  particular square.
 *
 *  @param x - horizontal ordinate
 *  @param y - vertical ordinate
 *
 *  @return 8-bit number - Normalized info about the box.
 */
uint8_t getNormalisedBoxVal(uint8_t x, uint8_t y){
  /* We want to normalize the values in the map boxes that correspond to the
   * 4 walls. To 'normalize' this value, we must bit shift the box values by the
   * amount of times the robot has rotated in the system.
   *
   * As long as the RotationFactor is either (0, 1, 2, 3) this algorithm will work.
   *
   * Example:
   *  - Consider we have map value = 1001 1001, with RotationFactor = 3
   *  - After normalization, the map value = 1100 1100
   *
   * We will use this example box value and RotationFactor in the below comments
   * to illustrate the algorithm.
   */
  uint8_t temp;
  uint8_t pwalls = (Map[x][y] & PWALLS);      //Eg. pwalls = 0000 1001
  uint8_t vwalls = (Map[x][y] & VWALLS) >> 4; //Eg. vwalls = 0000 1001

  vwalls = vwalls << PATH_RotationFactor;  //Eg. vwalls = 0100 1000
  temp = vwalls << 4;                      //Eg. temp = 1000 0000
  vwalls = (temp | vwalls) & VWALLS;       //Eg. ((1000 0000) | (0100 1000)) & WALL_MASK)
                                           //    = (1100 0000) Normalized box value!
  //Do the same for pwalls
  pwalls = pwalls << PATH_RotationFactor; //Eg. pwalls = 0100 1000
  temp = pwalls >> 4;                     //Eg. temp = 0000 01000
  pwalls = (temp | pwalls) & PWALLS;      //Eg. pwalls = 0000 1100

  return (vwalls | pwalls);               // return = 1100 1100
}

/*! @brief Finds and returns the highest 'flood' number in an accessible
 *         neighbouring cell.
 *
 *  @param x - horizontal ordinate
 *  @param y - vertical ordinate
 *
 *  @return 8-bit number - The highest 'flood' number found
 */
int8_t highestNeighbourCell(uint8_t x, uint8_t y){
  int8_t highestVal = -1;
  int8_t valAtNext;

  if(!(Map[x][y] & FRONT)){ //If there is not a wall in front of us on map
    valAtNext = PATH_Path[(x-1)][y];
    //If the value of that next box to the front of us has a value thats
    //greater than the biggest seen so far, then set it.
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  if(!(Map[x][y] & LEFT)){
    valAtNext = PATH_Path[x][(y-1)];
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  if(!(Map[x][y] & RIGHT)){
    valAtNext = PATH_Path[x][(y+1)];
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  if(!(Map[x][y] & BACK)){
    valAtNext = PATH_Path[(x+1)][y];
    if(valAtNext > highestVal)
      highestVal = valAtNext;
  }

  return highestVal;
}