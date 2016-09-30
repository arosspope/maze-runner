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
#define DRIVE_TOP_SPEED   500
#define BLIND_TOP_SPEED   300  //original 200 
#define DRIVE_TURN_SPEED  350
#define DRIVE_ROTATE_SPEED 210

/* Private function prototypes */
static void resetIRPos(void);
static void loadSongs(void);
static void playSong(uint8_t songNo);
bool moveForwardFrom(TORDINATE ord, TSENSORS * sens);
void updatePos(TORDINATE *ord);
void findNextSquare(TORDINATE currOrd);
int16_t getNextVal(TORDINATE currOrd);
bool victimCheck(TORDINATE vic1, TORDINATE vic2, TORDINATE curr);
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
  bool done = false;
  TSENSORS sens;
  TORDINATE home, currOrd, wayP1, wayP2, wayP3, wayP4, vic1, vic2;
  /* Initialise waypoints */
  home.x = 1; home.y = 3; //It is assumed that the robots start position is always (1,3)
  currOrd.x = 1; currOrd.y = 3;
  wayP1.x = 2; wayP1.y = 3;
  wayP2.x = 3; wayP2.y = 3;
  wayP3.x = 3; wayP3.y = 1;
  wayP4.x = 0; wayP4.y = 0;
  vic1.x = 6; vic1.y = 6;
  vic2.x = 6; vic2.y = 6;
  
  while(!(vic1Found && vic2Found) && !sensTrig)
  {
    PATH_Plan(currOrd, wayP1);
    while(!done)
    {
      LCD_PrintStr("WP1", BM_LEFT);
      victimCheck(vic1, vic2, currOrd);
      findNextSquare(currOrd);
      moveForwardFrom(currOrd, &sens);
      updatePos(&currOrd);
      
      done = ((currOrd.x == wayP1.x) && (currOrd.y == wayP1.y));
    }
    done = false;
    PATH_Plan(currOrd, wayP2);
    while(!done)
    {
      LCD_PrintStr("WP2", BM_LEFT);
      victimCheck(vic1, vic2, currOrd);
      findNextSquare(currOrd);
      moveForwardFrom(currOrd, &sens);
      updatePos(&currOrd);
      
      done = ((currOrd.x == wayP2.x) && (currOrd.y == wayP2.y));
    }
    done = false;
    PATH_Plan(currOrd, wayP3);
    while(!done)
    {
      LCD_PrintStr("WP3", BM_LEFT);
      victimCheck(vic1, vic2, currOrd);
      findNextSquare(currOrd);
      moveForwardFrom(currOrd, &sens);
      updatePos(&currOrd);
      
      done = ((currOrd.x == wayP3.x) && (currOrd.y == wayP3.y));
    }
    done = false;
    PATH_Plan(currOrd, wayP4);
    while(!done)
    {
      LCD_PrintStr("WP4", BM_LEFT);
      victimCheck(vic1, vic2, currOrd);
      findNextSquare(currOrd);
      moveForwardFrom(currOrd, &sens);
      updatePos(&currOrd);
      
      done = ((currOrd.x == wayP4.x) && (currOrd.y == wayP4.y));
    }
    done = false;
  }
  done = false;
  PATH_Plan(currOrd, home);
  while(!done)
  {
    LCD_PrintStr("Home", BM_LEFT);
    findNextSquare(currOrd);
    moveForwardFrom(currOrd, &sens);
    updatePos(&currOrd);

    done = ((currOrd.x == home.x) && (currOrd.y == home.y));
  }
}

bool IROBOT_WallFollow(TDIRECTION irDir, int16_t moveDist){
  double tolerance, dist;
  TSENSORS sens;
  bool triggered = false;
  uint16_t orientation = SM_Move(0, DIR_CW);
  int16_t distmoved = 0;
  
  //Reset IR position then face IR sensor 45 degs in particular direction, if its
  //already at that position - dont do anything
  if(irDir == DIR_CW){
    if(orientation != 25){
      resetIRPos();
      SM_Move(25, irDir);
    }
  }else{
    if(orientation != 175){
      resetIRPos();
      SM_Move(25, irDir);
    }
  }
  
  //Get current distance reading, and set tolerance (i.e. distance from wall) at 0.707m
  tolerance = 650;  
  dist = IR_Measure();
  MOVE_GetDistMoved();      //Reset the distance moved encoders on the iRobot
  
  while ((distmoved < moveDist) && !triggered){ //While the distance traveled is less than required
    if(irDir == DIR_CCW) //Left-Wall follow
    {
      if((dist < (tolerance + 3)) && (dist > (tolerance - 3))){  //while IR reading is between plus/minus 5mm execute following lines
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);      //left wheel and right wheel drive same speed.
      }
      else if (dist < (tolerance - 3)){      //Robot to close to wall
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
      else{                                  //Robot to far away from wall
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
    }
    else //Right-wall follow
    {
      if((dist < (tolerance + 3)) && (dist > (tolerance - 3))){ 
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);
      }
      else if (dist < (tolerance - 3)){
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
      else{
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
    }
    
    dist = IR_Measure();         //Keep checking wall distance.
    distmoved += MOVE_GetDistMoved();
    triggered |= MOVE_CheckSensor(&sens);
  }
  
  MOVE_DirectDrive(0,0);  //Stop iRobot
  return triggered;
}

/*! @brief Resets the position of the IR sensor back to 0 (forward facing).
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
 *  @param ord - The x/y coordinate to move forward from
 * 
 *  @note Assumes the robot has already been orientated to move into the next 
 *        grid location.
 */
bool moveForwardFrom(TORDINATE ord, TSENSORS * sens){
  bool triggered = false;
  int blind_driveForwardDist = 430;  //original 480
  int wall_driveForwardDist = 500;
    
  TORDINATE nextOrd;
  nextOrd.x = ord.x; nextOrd.y = ord.y;
  
  //Calculates the next grid position based on what way the robot is facing
  switch(PATH_RotationFactor){
      case 0:
        nextOrd.x = ord.x - 1; break;
      case 1:
        nextOrd.y = ord.y + 1; break;
      case 2:
        nextOrd.x = ord.x + 1; break;
      case 3:
        nextOrd.y = ord.y - 1; break;
  }

  if(PATH_GetMapInfo(ord, BOX_Left) && PATH_GetMapInfo(nextOrd, BOX_Left))
  { 
    //If we can follow a wall on the left for 1m do it.
    IROBOT_WallFollow(DIR_CCW, wall_driveForwardDist);
    if(PATH_GetMapInfo(nextOrd, BOX_Front) )
    {
      //If the next cube has a wall in front of us, make sure we don't hit it.
      double ir,dist;
      resetIRPos();   //Face the IR forward
      ir = IR_Measure();
      dist = 500;
      while(ir>dist)   //Drive straight until we are 0.5m from the wall
      {
        MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0);
    }
    else
      IROBOT_WallFollow(DIR_CCW, wall_driveForwardDist);
  } 
  else if (PATH_GetMapInfo(ord, BOX_Right) && PATH_GetMapInfo(nextOrd, BOX_Right))
  {
    //If we can follow a wall on the right for 1m do it
    IROBOT_WallFollow(DIR_CW, wall_driveForwardDist);
    if(PATH_GetMapInfo(nextOrd, BOX_Front) )
    {
      //If the next cube has a wall in front of us, make sure we don't hit it.
      double ir,dist;
      resetIRPos();   //Face the IR forward
      ir = IR_Measure();
      dist = 500;
      while(ir>dist)   //Drive straight until we are 0.5m from the wall
      {
          MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
          ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0);
    }
    else
      IROBOT_WallFollow(DIR_CW, wall_driveForwardDist);
  } 
  else if (!PATH_GetMapInfo(ord, BOX_Left) && PATH_GetMapInfo(nextOrd, BOX_Left))
  {
    //If there's not a wall to the left of us in this box, but there is one in the next
    //box, then drive straight half-way, then use a wall follow the rest of the way
    triggered |= MOVE_Straight(BLIND_TOP_SPEED, blind_driveForwardDist, sens);
    IROBOT_WallFollow(DIR_CCW, 500);
  }
  else if (!PATH_GetMapInfo(ord, BOX_Right) && PATH_GetMapInfo(nextOrd, BOX_Right))
  {
    //Same as above but for the right wall
    triggered |= MOVE_Straight(BLIND_TOP_SPEED, blind_driveForwardDist, sens);
    IROBOT_WallFollow(DIR_CW, 500);
  }
  else
  {
    //Else just drive straight
    triggered |= MOVE_Straight(BLIND_TOP_SPEED, blind_driveForwardDist, sens);
    if(PATH_GetMapInfo(nextOrd, BOX_Front) )
    {
      //If the next cube has a wall in front of us, make sure we don't hit it.
      double ir,dist;
      resetIRPos();   //Face the IR forward
      ir = IR_Measure();
      dist = 500;
      while(ir>dist)   //Drive straight until we are 0.5m from the wall
      {
          MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
          ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0);
    }
    else
      triggered |= MOVE_Straight(BLIND_TOP_SPEED, blind_driveForwardDist, sens);
  }
  
  return triggered;
}

/*! @brief Gets the flood fill value of the box in front of the robots 
 *         virtual position.
 *  
 *  @param currOrd - The current virtual position of the robot. 
 *  @return Flood fill value of the square in front of the robot
 */
int16_t getNextVal(TORDINATE currOrd){
  switch(PATH_RotationFactor){
      case 0:
        currOrd.x = currOrd.x - 1; break;
      case 1:
        currOrd.y = currOrd.y + 1; break;
      case 2:
        currOrd.x = currOrd.x + 1; break;
      case 3:
        currOrd.y = currOrd.y - 1; break;
    }
  
  return PATH_Path[currOrd.x][currOrd.y];
}

/*! @brief Finds the next square to move into (based on flood fill) and orient
 *         the robot to face that square.
 * 
 *  @param currOrd - The box to move from
 */
void findNextSquare(TORDINATE currOrd){
  bool frontLowest = false;
  bool rightLowest = false;
  bool leftLowest = false;
  bool behindLowest = false;
  int16_t lowestSoFar = PATH_Path[currOrd.x][currOrd.y];
  int16_t temp;
  
  if(!PATH_GetMapInfo(currOrd, BOX_Front)){
    temp = getNextVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      frontLowest = true;
      lowestSoFar = temp;
    }
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Left)){
    PATH_UpdateOrient(1, DIR_CCW); //Virtually turn the robot
    temp = getNextVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      frontLowest = false;
      leftLowest = true;
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(1, DIR_CW); //Virtually rest the robot
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Right)){
    PATH_UpdateOrient(1, DIR_CW); //Virtually turn the robot
    temp = getNextVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      frontLowest = false;
      leftLowest = false;
      rightLowest = true;
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(1, DIR_CCW); //Virtually rest the robot
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Back)){
    PATH_UpdateOrient(2, DIR_CCW); //Virtually turn the robot
    temp = getNextVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      frontLowest = false;
      leftLowest = false;
      rightLowest = false;
      behindLowest = true;
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(2, DIR_CW); //Virtually rest the robot
  }
  
  if(frontLowest){
    //Do Nothing to rotate
  } else if (leftLowest){
    MOVE_Rotate(DRIVE_ROTATE_SPEED, 87, DIR_CCW);
    PATH_UpdateOrient(1, DIR_CCW);
  } else if (rightLowest){
    MOVE_Rotate(DRIVE_ROTATE_SPEED, 87, DIR_CW);
    PATH_UpdateOrient(1, DIR_CW);
  } else if (behindLowest){
    MOVE_Rotate(DRIVE_ROTATE_SPEED, 180, DIR_CCW);
    PATH_UpdateOrient(2, DIR_CCW);
  }
  
}

/*! @brief Updates the coordinate position based on the robots forward facing direction.
 *   
 *  @param ord - The ordinate to update
 */
void updatePos(TORDINATE *ord){
  switch(PATH_RotationFactor){
      case 0:
        ord->x = ord->x - 1; break;
      case 1:
        ord->y = ord->y + 1; break;
      case 2:
        ord->x = ord->x + 1; break;
      case 3:
        ord->y = ord->y - 1; break;
    }
}

bool victimCheck(TORDINATE vic1, TORDINATE vic2, TORDINATE curr){
  bool bothVicsFound = false;
  uint8_t rxdata;
          
  USART_OutChar(OP_SENSORS);
  USART_OutChar(OP_SENS_IR);
  rxdata = USART_InChar();
  
  if(rxdata == 254 || rxdata == 242 || rxdata == 250 || rxdata == 256){
    if(vic1.x == 6 && vic1.y == 6){
      vic1.x = curr.x; vic1.y = curr.y;
      playSong(0);
    }
    else if((vic2.x == 6 && vic2.y == 6) && !(vic1.x == curr.x && vic1.y == curr.y)){
      vic2.x = curr.x; vic2.y = curr.y;
      playSong(2);
      bothVicsFound = true;
    }
  }
  
  return bothVicsFound;
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