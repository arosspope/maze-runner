/*! @file IROBOT.c
 *
 *  @brief Module to control the iRobot.
 *
 *  This contains the functions for communicating with the iRobot through the
 *  create open interface.
 *
 *  @author A.Pope, K.Leone, C.Stewart, J.Lynch
 *  @date 02-09-2016
 */
#include "IR.h" //TODO: Ensure IR is being initialised here
#include "EEPROM.h"
#include "USART.h"
#include "PATH.h"
#include "MOVE.h"
#include "SM.h"
#include "LCD.h" //TODO: Remove references to LCD
#include "OPCODES.h"
#include "IROBOT.h"

/* Loading Song Notes to EEPROM (pre-loading only) - TODO: Choose notes */
//Note duration is stored next to the note:
//  -e.g. DATA(D, 0.5s, C#, 0.2s); 'D' will play for 0.5seconds, 'C#' will play for 0.2s
__EEPROM_DATA(71, 32, 76, 31, 79, 31, 78, 33);  //Song 0 - ADDR offset: 0x00 Harry Potter-Victim 1
__EEPROM_DATA(76, 34, 71, 33, 69, 31, 78, 34);
__EEPROM_DATA(98, 30, 103, 30, 110, 25, 108, 10);  //Song 1 - ADDR offset: 0x10 Star Wars-Victim 2
__EEPROM_DATA(107, 10, 105, 10, 115, 25, 110, 20);
__EEPROM_DATA(100, 10, 100, 10, 100, 10, 96, 20);          //Song 2 - ADDR offset: 0x20 Mario-Finish 
__EEPROM_DATA(100, 20, 103, 20, 91, 30, 88, 20);
__EEPROM_DATA(1, 2, 3, 4, 5, 6, 7, 8)  ;          //Song 3 - ADDR offset: 0x30
__EEPROM_DATA(9, 10, 11, 12, 13, 14, 15, 16);

/* Loading Map data to EEPROM; ADDR offset 0x40 - TODO: Must do */

//Optimal speeds for driving the iROBOT
#define DRIVE_TOP_SPEED   250
#define DRIVE_TURN_SPEED  150
#define DRIVE_ROTATE_SPEED 210

/* Private function prototypes */
static void wallAlign(uint16_t wallLocation);
static void resetIRPos(void);
static void loadSongs(void);
static void playSong(uint8_t songNo);
static uint16_t getAlignAngle(uint8_t x, uint8_t y, TDIRECTION dir, uint8_t numTurns);
static uint16_t closestObject(void);
/* End Private function prototypes */

bool IROBOT_Init(void){
  return (USART_Init() && IR_Init() && SM_Init() && MOVE_Init() && PATH_Init());
}

void IROBOT_Start(void){
  //Put the IROBOT in full control mode and load songs
  USART_OutChar(OP_START);
  USART_OutChar(OP_FULL);
  loadSongs();
}

//TODO: Must remove - test routine, used for testing specific bits of code.
void IROBOT_Test(void){
  MOVE_Rotate(210, 90, DIR_CW);
  playSong(0);
}

void goStraight(uint8_t x, uint8_t y){
  uint8_t xNext = x;
  uint8_t yNext = y;
  
  switch(PATH_RotationFactor){
      case 0:
        xNext = x - 1; break;
      case 1:
        yNext = y + 1; break;
      case 2:
        xNext = x + 1; break;
      case 3:
        yNext = y - 1; break;
  }
  
  LCD_PrintInt(PATH_RotationFactor, TOP_LEFT);
  LCD_PrintInt(xNext, BM_LEFT);
  LCD_PrintInt(yNext, BM_RIGHT);
  
  if(PATH_GetMapInfo(x, y, BOX_Left) && PATH_GetMapInfo(xNext, yNext, BOX_Left))
  {
    IROBOT_WallFollow(true, 1000);
  } 
  else if (PATH_GetMapInfo(x, y, BOX_Right) && PATH_GetMapInfo(xNext, yNext, BOX_Right))
  {
    IROBOT_WallFollow(false, 1000);
  } 
//  else if (PATH_GetMapInfo(x, y, BOX_Left) && !PATH_GetMapInfo(xNext, yNext, BOX_Left))
//  {
//    IROBOT_WallFollow(true, 500);
//    MOVE_Straight(DRIVE_TOP_SPEED, 500);
//  }
  else if (!PATH_GetMapInfo(x, y, BOX_Left) && PATH_GetMapInfo(xNext, yNext, BOX_Left))
  {
    MOVE_Straight(DRIVE_TOP_SPEED, 500);
    IROBOT_WallFollow(true, 500);
  }
//  else if (PATH_GetMapInfo(x, y, BOX_Right) && !PATH_GetMapInfo(xNext, yNext, BOX_Right)){
//    IROBOT_WallFollow(false, 500);
//    MOVE_Straight(DRIVE_TOP_SPEED, 500);
//  }
  else if (!PATH_GetMapInfo(x, y, BOX_Left) && PATH_GetMapInfo(xNext, yNext, BOX_Right)){
    MOVE_Straight(DRIVE_TOP_SPEED, 500);
    IROBOT_WallFollow(false, 500);
  }
  else
  {
    MOVE_Straight(DRIVE_TOP_SPEED, 1000);
  }
  
  
  
}


void IROBOT_MazeRun(void){
  bool vic1Found = false; bool vic2Found = false; bool sensTrig = false;
  uint16_t theta;
  uint8_t x = 1; //It is assumed that the robots start position is always (1,3)
  uint8_t y = 3;
  
  while(!(vic1Found && vic2Found) && !sensTrig)
  {
    if(!PATH_GetMapInfo(x, y, BOX_Left))
    {
      MOVE_Rotate(DRIVE_ROTATE_SPEED, 90, DIR_CCW);
      PATH_UpdateOrient(1, DIR_CCW);      
      goStraight(x, y);
    } 
    else if (!PATH_GetMapInfo(x, y, BOX_Front))
    {
      goStraight(x, y);
    } 
    else if (!PATH_GetMapInfo(x, y, BOX_Right))
    {
      MOVE_Rotate(DRIVE_ROTATE_SPEED, 90, DIR_CW);
      PATH_UpdateOrient(1, DIR_CW);
      goStraight(x, y);
    } else {
      MOVE_Rotate(DRIVE_ROTATE_SPEED, 180, DIR_CW);
      PATH_UpdateOrient(2, DIR_CW);      
      goStraight(x, y);
    }
    
    switch(PATH_RotationFactor){
      case 0:
        x = x - 1; break;
      case 1:
        y = y + 1; break;
      case 2:
        x = x + 1; break;
      case 3:
        y = y - 1; break;
    }
    
//    /* For the current box we are in, determine the next possible location to move
//     * in the following preference heirachy:
//     *  1. Left, 2. Front, 3. Right
//     * If there is not a wall blocking us there, then go that direction.
//     */
  }
}

void IROBOT_WallFollow(bool leftWallFollow, int16_t moveDist){
  double tolerance, dist;    //dist variable stores the IR sensor reading
  SensorsStatus_t sensStatus;
  int16_t distmoved = 0;
  
  //Reset IR position, find the closest wall (for left-wall follow) and align
  resetIRPos();
  
  if(leftWallFollow)
    SM_Move(25, DIR_CCW);
  else
    SM_Move(25, DIR_CW);
  
  //Store current distance reading at a 45 degree angle to the wall from IR sensor
  tolerance = 707;  
  dist = IR_Measure();
  MOVE_GetDist();
  
  while ((distmoved < moveDist)){  //if no sensors have been triggered i.e. bumper
    if(leftWallFollow)
    {
      if((dist < (tolerance + 5)) && (dist > (tolerance - 5))){  //while IR reading is between plus/minus 5mm execute following lines
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);          //left wheel and right wheel drive same speed.
      }
      else if (dist < (tolerance - 5)){      //Robot to close to wall
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
      else{                                  //Robot to far away from wall
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
    }
    else 
    {
      if((dist < (tolerance + 5)) && (dist > (tolerance - 5))){  //while IR reading is between plus/minus 5mm execute following lines
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);      //left wheel and right wheel drive same speed.
      }
      else if (dist < (tolerance - 5)){      //Robot to close to wall
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
      else{                                  //Robot to far away from wall
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
    }
    
    dist = IR_Measure();                   //Keep checking wall distance.
    distmoved += MOVE_GetDist();
    //distmoved += 1;
  }
  
  MOVE_DirectDrive(0,0);  //if sensor is triggered, stop the IROBOT
}

/* @brief Will rotate the robot and align the robot parallel to the closest wall. 
 *
 * @param wallLocation - The location (in steps) of the closest wall to follow.
 */
static void wallAlign(uint16_t wallLocation){
  uint16_t moveAngle;

  if(wallLocation >= 150){ //greater than 270 degrees (from 0 CW)
    moveAngle = (uint16_t) ((wallLocation * SM_F_STEP_RESOLUTION) - 270);
    MOVE_Rotate(DRIVE_ROTATE_SPEED, moveAngle, DIR_CW); //Rotate robot so its parrallel with the wall

    //Calculate steps required to get IR sensor pointing 45 degs from 360 (CCW)
    if(moveAngle <= 45){
      SM_Move((uint16_t)((45 - moveAngle)/SM_F_STEP_RESOLUTION), DIR_CW);
    }else{ //Its greater than 45 degs
      SM_Move((uint16_t)((moveAngle - 45)/SM_F_STEP_RESOLUTION), DIR_CCW);
    }

  } 
  else if ((wallLocation <= 150)){
    moveAngle = (uint16_t)(270 - (wallLocation * SM_F_STEP_RESOLUTION));
    MOVE_Rotate(DRIVE_ROTATE_SPEED, moveAngle, DIR_CCW);
    SM_Move((uint16_t)((moveAngle + 45) / SM_F_STEP_RESOLUTION), DIR_CW);
  }
}

/*! @brief Performs a 360 scan of the environment using the IR sensor. At the
 *  completion of the scan, the sensor will pointed towards the closest object.
 *
 *  @return position - The orientation of the closest object (in steps).
 */
static uint16_t closestObject(void){
  uint16_t i, orientation, closestObject, stepsBack, offset;
  uint16_t stepsFor360 = SM_F_STEPS_FOR_180 * 2;
  double smallestIR = 4000.0; //Set this initial value to outside the range of the IR sensor
  double data;

  //Move the Stepper motor 0 steps to obtain the current orientation
  orientation = SM_Move(0, DIR_CW);
  offset = orientation; //The offset equals the current orientation

  //Move stepper motor 360 degs, taking IR samples each time
  for(i = 0; i < stepsFor360; i++){
    data = IR_Measure();            //Measure the distance

    if (data < smallestIR)
    {
      smallestIR = data;            //Update the data to smallest reading
      closestObject = orientation;  //Update the step at which the closest object was found
    }

    orientation = SM_Move(1, DIR_CW);   //Move the motor half a step CW
    LCD_PrintInt((int) data, TOP_RIGHT);  //Print the IR reading to LCD - TODO: Test code
  }

  //Calculate the amount of steps required to point sensor back to the closest object
  stepsBack = ((stepsFor360 - 1) + offset - closestObject) % stepsFor360;
  return SM_Move(stepsBack, DIR_CCW);
}

/*! @brief Resets the postion of the IR sensor back to 0 (forward facing).
 *
 */
static void resetIRPos(void){
  uint16_t orientation = SM_Move(0, DIR_CW);  //Get where the IR is pointing
  uint16_t moveBack;
  
  if(orientation > 100){
    moveBack = 200 - orientation;
    SM_Move(moveBack, DIR_CW);
  } else {
    SM_Move(orientation, DIR_CCW);
  }
  
  //SM_Move(orientation, DIR_CCW);              //Move back those number of steps
}

/*! @brief Loads the 4 pre-defined songs onto the iRobot
 *
 */
static void loadSongs(void){
  uint8_t i, j, addrOffset = 0;

  //Load the four songs on the iRobot Create
  for(i = 0; i < 4; i++)
  {
    USART_OutChar(OP_LOAD_SONG);
    USART_OutChar(i);                   //Song number
    USART_OutChar(EEPM_NUM_SONG_NOTES); //Notes in a song

    for(j = 0; j < EEPM_SONG_MEM_SIZE; j+= 2)
    {
      USART_OutChar(eeprom_read((addrOffset + j)));   //Load the note stored in flash mem
      USART_OutChar(eeprom_read((addrOffset + j)+1)); //Note duration is stored next to the note in mem 
    }

    addrOffset += EEPM_SONG_MEM_SIZE; //Increment the address offset for the next song
  }
}

/*! @brief Loads the 4 pre-defined songs onto the iRobot
 *
 *  @param songNo - The song to play (0 - 3)
 */
static void playSong(uint8_t songNo){
  bool songPlaying;

  //Wait until previous song has finished playing
  do {
    USART_OutChar(OP_SENSORS);
    USART_OutChar(OP_SONG_PLAYING);
    songPlaying = USART_InChar();
  } while(songPlaying);

  USART_OutChar(OP_PLAY_SONG);
  USART_OutChar(songNo);
}

/*! @brief Calculates the desired angle to rotate a certain amount of 90 degree
 *         turns based on the current 'error' in relation to the map.
 *
 *  @param x - coordinate x on the map
 *  @param y - coordinate y on the map
 *  @param dir - Direction to turn in
 *  @param numTurns - The amount of 90 degs turns to perform
 *
 *  @return theta - The angle to move in the specified direction
 */
static uint16_t getAlignAngle(uint8_t x, uint8_t y, TDIRECTION dir, uint8_t numTurns){
  uint16_t i, closestWall, theta, orientation;
  double smallestIR = 4000.0; //Set this initial value to outside the range of the IR sensor
  double data;

  //Reset the IR position to 0 forward facing the robot
  resetIRPos();

  //Preferrably, we would like to align ourselves on a wall in front of us
  if(PATH_GetMapInfo(x, y, BOX_Front))
  {
    //Move the IR sensor 45 CCW (25 steps)
    orientation = SM_Move(25, DIR_CCW);

    //Do a 90 degree sweep (50 steps) back CW, and find the angle (step) at which the wall was closest
    for(i = 0; i < 50; i++){
      data = IR_Measure();
      if (data < smallestIR){
        smallestIR = data;
        closestWall = orientation;
      }
      orientation = SM_Move(1, DIR_CW);
    }

    //Normalise wall so that it is in degrees (steps -> degrees)
    closestWall = (uint16_t) (closestWall * SM_F_STEP_RESOLUTION);

    if(dir == DIR_CCW) //Left-movement
    {
      if(closestWall < 90){
        theta = (90*numTurns) - closestWall;
      } else {
        theta = (90*numTurns) + (360 - closestWall);
      }
    }
    else //Right-movement
    {
      if(closestWall < 90){
        theta = (90*numTurns) + closestWall;
      } else {
        theta = (90*numTurns) - (360 - closestWall);
      }
    }
  }
  else
  {
    //There is no wall - we hope for the best. TODO: there must be a better method?
    theta = (90*numTurns);
  }

  return theta;
}