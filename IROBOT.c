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
static void resetIRPos(void);
static void loadSongs(void);
static void playSong(uint8_t songNo);
void moveForwardFrom(uint8_t x, uint8_t y);
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
      moveForwardFrom(x, y);
    } 
    else if (!PATH_GetMapInfo(x, y, BOX_Front))
    {
      moveForwardFrom(x, y);
    } 
    else if (!PATH_GetMapInfo(x, y, BOX_Right))
    {
      MOVE_Rotate(DRIVE_ROTATE_SPEED, 90, DIR_CW);
      PATH_UpdateOrient(1, DIR_CW);
      moveForwardFrom(x, y);
    } else {
      MOVE_Rotate(DRIVE_ROTATE_SPEED, 180, DIR_CW);
      PATH_UpdateOrient(2, DIR_CW);      
      moveForwardFrom(x, y);
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
  }
}

void IROBOT_WallFollow(TDIRECTION irDir, int16_t moveDist){
  double tolerance, dist;
  SensorsStatus_t sensStatus;
  int16_t distmoved = 0;
  
  //Reset IR position then face IR sensor 45 degs in particular direction
  //(i.e. left or right wall follow)
  resetIRPos();
  SM_Move(25, irDir);
  
  //Get current distance reading, and set tolerance (i.e. distance from wall) at 0.707m
  tolerance = 707;  
  dist = IR_Measure();
  MOVE_GetDistMoved();      //Reset the distance moved encoders on the iRobot
  
  while ((distmoved < moveDist)){ //While the distance traveled is less than required
    if(irDir == DIR_CCW) //Left-Wall follow
    {
      if((dist < (tolerance + 5)) && (dist > (tolerance - 5))){  //while IR reading is between plus/minus 5mm execute following lines
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);      //left wheel and right wheel drive same speed.
      }
      else if (dist < (tolerance - 5)){      //Robot to close to wall
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
      else{                                  //Robot to far away from wall
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
    }
    else //Right-wall follow
    {
      if((dist < (tolerance + 5)) && (dist > (tolerance - 5))){ 
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);
      }
      else if (dist < (tolerance - 5)){
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
      else{
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
    }
    
    dist = IR_Measure();         //Keep checking wall distance.
    distmoved += MOVE_GetDistMoved();
  }
  
  MOVE_DirectDrive(0,0);  //Stop iRobot
}

/*! @brief Resets the postion of the IR sensor back to 0 (forward facing).
 *
 */
static void resetIRPos(void){
  uint16_t orientation = SM_Move(0, DIR_CW);  //Get where the IR is pointing
  
  if(orientation > 100){ //If IR sensor is past 180 degs
    orientation = 200 - orientation;
    SM_Move(orientation, DIR_CW);
  } else {
    SM_Move(orientation, DIR_CCW);
  }
}

/*! @brief Will determine the best way to move forward into the next square, 
 *         from its current position. 
 *
 *  @param x - Horizontal coordinate of current location.
 *  @param y - Vertical coordinate of current location.
 * 
 *  @note Assumes the robot has already been orientated to move into the next 
 *        grid location.
 */
void moveForwardFrom(uint8_t x, uint8_t y){
  uint8_t xNext = x;
  uint8_t yNext = y;
  
  //Calculates the next grid position based on what way the robot is facing
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
  
  if(PATH_GetMapInfo(x, y, BOX_Left) && PATH_GetMapInfo(xNext, yNext, BOX_Left))
  {
    //If we can follow a wall on the left for 1m do it.
    IROBOT_WallFollow(DIR_CCW, 1000);
  } 
  else if (PATH_GetMapInfo(x, y, BOX_Right) && PATH_GetMapInfo(xNext, yNext, BOX_Right))
  {
    //If we can follow a wall on the right for 1m do it
    IROBOT_WallFollow(DIR_CW, 1000);
  } 
  else if (!PATH_GetMapInfo(x, y, BOX_Left) && PATH_GetMapInfo(xNext, yNext, BOX_Left))
  {
    //If there's not a wall to the left of us in this box, but there is one in the next
    //box, then drive straight half-way, then use a wall follow the rest of the way
    MOVE_Straight(DRIVE_TOP_SPEED, 500);
    IROBOT_WallFollow(DIR_CCW, 500);
  }
  else if (!PATH_GetMapInfo(x, y, BOX_Right) && PATH_GetMapInfo(xNext, yNext, BOX_Right))
  {
    //Same as above but for the right wall
    MOVE_Straight(DRIVE_TOP_SPEED, 500);
    IROBOT_WallFollow(DIR_CW, 500);
  }
  else
  {
    //Else just drive straight
    MOVE_Straight(DRIVE_TOP_SPEED, 1000);
  }
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