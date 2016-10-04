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
#define DRIVE_TOP_SPEED   400  //ORIG 500
#define BLIND_TOP_SPEED   300  //original 200 
#define DRIVE_TURN_SPEED  330  //CHANGING IT TO 300 MADE IT UNDERSHOOT 
#define DRIVE_ROTATE_SPEED 210
#define ANGLE_ER 3

/* Private function prototypes */
static void resetIRPos(void);
static void loadSongs(void);
static void playSong(uint8_t songNo);
bool moveForwardFrom(TORDINATE ord, TSENSORS * sens, int16_t * movBack);
void updatePos(TORDINATE *ord);
void findNextSquare(TORDINATE currOrd);
int16_t getNextPathVal(TORDINATE currOrd);
bool areAllVictimsFound(TORDINATE curr);
bool victimFound(void);
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
  playSong(0);
}

void errorHandle(TORDINATE ord, TSENSORS sensor, int16_t movBack){
  if(sensor.bump){
    MOVE_Straight(-350, movBack, false, 0, 0);
  }
  
  if(sensor.wall){
    
  }
}

void IROBOT_MazeRun(void){
  bool bothVicsFound = false; bool sensTrig = false; bool triggered = false; bool flip = true;
  TSENSORS sens; uint8_t i = 0; int16_t movBack = 0;
  /* Initialise waypoints */
  TORDINATE home = {1, 3};
  TORDINATE currOrd = {1, 3};
  TORDINATE wayP1 = {2, 3};
  TORDINATE wayP2 = {3, 3};
  TORDINATE wayP3 = {3, 1};
  TORDINATE wayP4 = {0, 0};
  /* Iniitialise the list of waypoints */
  TORDINATE wayList[4];
  wayList[0] = wayP1; wayList[1] = wayP2; wayList[2] = wayP3; wayList[3] = wayP4;

  while(!bothVicsFound && !sensTrig){
    //Loop through WayPoint List and continue to move around the maze, until
    //both victims are found
    PATH_Plan(currOrd, wayList[i]);
    
    while(!(currOrd.x == wayList[i].x && currOrd.y == wayList[i].y)) //While we havent gotten to the current waypoint
    {
      bothVicsFound = areAllVictimsFound(currOrd); //Check square for victims and determine if all have been found
      if(bothVicsFound)
        break; //Break inner while loop and go home
      
      findNextSquare(currOrd);            //Find next sqaure to move to, and rotate robot to face
      if(moveForwardFrom(currOrd, &sens, &movBack)) //If a Sensor was triggered
      {
        errorHandle(currOrd, sens, movBack);
      } else {
        updatePos(&currOrd); //Everything was fine, update position
      }
      movBack = 0;
    }

    i = (i + 1) % 4; //Make sure to move within the waypoint list
  }

  //We have found both victims, time to go home!
  PATH_Plan(currOrd, home);
  while(!(currOrd.x == home.x && currOrd.y == home.y)) //While we havent gotten to the waypoint
  {
    findNextSquare(currOrd);
    if(moveForwardFrom(currOrd, &sens, &movBack)){
      errorHandle(currOrd, sens, movBack);
    } else {
      updatePos(&currOrd);
    }
    movBack = 0;
  }
}

bool IROBOT_WallFollow(TDIRECTION irDir, TSENSORS * sens, int16_t moveDist, int16_t * movBack){
  double tolerance = 700; //Ensure we stay 650mm from the wall
  bool triggered = false;
  uint16_t orientation = SM_Move(0, DIR_CW);
  int16_t distmoved = 0;
  double dist;
 
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
  dist = IR_Measure();
  MOVE_GetDistMoved();      //Reset the distance moved encoders on the iRobot
  
  while ((distmoved < moveDist) && !triggered){ //While the distance traveled is less than required
    if(irDir == DIR_CCW) //Left-Wall follow
    {
      if((dist < (tolerance + 2)) && (dist > (tolerance - 2))){  //while IR reading is between plus/minus 5mm execute following lines
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);      //left wheel and right wheel drive same speed.
      }
      else if (dist < (tolerance - 2)){      //Robot to close to wall
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
      else{                                  //Robot to far away from wall
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
    }
    else //Right-wall follow
    {
      if((dist < (tolerance + 2)) && (dist > (tolerance - 2))){ 
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);
      }
      else if (dist < (tolerance - 2)){
        MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
      }
      else{
        MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
      }
    }
    
    dist = IR_Measure();                //Keep checking wall distance.
    distmoved += MOVE_GetDistMoved();   //Get Distance moved since last call
    triggered = MOVE_CheckSensor(sens);//Check sensors
  }
  
  MOVE_DirectDrive(0,0);  //Stop iRobot
  
  if(triggered)
    *movBack += distmoved;
  
  return triggered;
}

/*! @brief Will determine the best way to move forward into the next square, 
 *         from its current position. 
 *
 *  @param ord - The x/y coordinate to move forward from
 * 
 *  @note Assumes the robot has already been orientated to move into the next 
 *        grid location.
 */
bool moveForwardFrom(TORDINATE ord, TSENSORS * sens, int16_t * movBack){
  bool triggered = false;
  bool LWallF, RWallF, LHWallF, RHWallF, FInNext, BWall;
  double ir;
  int temp;
  int blind_driveForwardDist = 500;  //original 480
  int wall_driveForwardDist = 450;
  TORDINATE nextOrd = ord;
  
  //Calculates the next grid position based on what way the robot is facing
  updatePos(&nextOrd);
  //Get case information, to decide how to move forward
  LWallF  = PATH_GetMapInfo(ord, BOX_PLeft) && PATH_GetMapInfo(nextOrd, BOX_PLeft);
  RWallF  = PATH_GetMapInfo(ord, BOX_PRight) && PATH_GetMapInfo(nextOrd, BOX_PRight);
  LHWallF = !PATH_GetMapInfo(ord, BOX_PLeft) && PATH_GetMapInfo(nextOrd, BOX_PLeft);
  RHWallF = !PATH_GetMapInfo(ord, BOX_PRight) && PATH_GetMapInfo(nextOrd, BOX_PRight);  
  FInNext = PATH_GetMapInfo(nextOrd, BOX_PFront);
  BWall = PATH_GetMapInfo(ord, BOX_PBack);
  
  if(LWallF || RWallF) //If we can follow a wall on the left/right for 1m
  { 
    if(FInNext && !triggered)
    {
      if(LWallF)
        triggered = IROBOT_WallFollow(DIR_CCW, sens, 500, movBack);
      else
        triggered = IROBOT_WallFollow(DIR_CW, sens, 500, movBack);
      
      if(!triggered)
      {
        resetIRPos(); ir = IR_Measure(); //Face the IR forward and get reading
        MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
        while((ir > 500) && !triggered)  //Drive straight until we are 0.5m from the wall
        {
          triggered = MOVE_CheckSensor(sens);
          ir = IR_Measure();
        }
        MOVE_DirectDrive(0,0); //Stop the robot
        
        if(triggered)
          *movBack += (ir - 500) + 500; //Calculate dist required to move Back
      }
    }
    else if(BWall && !triggered){
      //If the next cube has a wall in front of us, make sure we don't hit it.
      resetIRPos(); SM_Move(100, DIR_CW); ir = IR_Measure(); //Face the IR forward and get reading
      temp = (int16_t)ir;
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
      while((ir < 900) && !triggered)  //Drive straight until we are 0.5m from the wall
      {
        triggered = MOVE_CheckSensor(sens);
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered)
        *movBack += (ir - temp); //Calculate dist required to move Back
      
      if(LWallF && !triggered)
        triggered = IROBOT_WallFollow(DIR_CCW, sens, 600, movBack);
      else if(RWallF && !triggered)
        triggered = IROBOT_WallFollow(DIR_CW, sens, 600, movBack);
    }
    else if(!BWall && !FInNext && !triggered){
      if(LWallF)
        triggered = IROBOT_WallFollow(DIR_CCW, sens, 1000, movBack);
      else
        triggered = IROBOT_WallFollow(DIR_CW, sens, 1000, movBack);
    }
  } 
  else if(LHWallF || RHWallF) //If we can follow a wall for half-the way
  {
    //If there's not a wall to the left/right of us in this box, but there is one in the next
    //box, then drive straight half-way, then use a wall follow the rest of the way
    
    if(BWall && !triggered){
      //If the next cube has a wall in front of us, make sure we don't hit it.
      resetIRPos(); SM_Move(100, DIR_CW); ir = IR_Measure(); //Face the IR forward and get reading
      temp = (int16_t)ir;
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
      while((ir < 900) && !triggered)  //Drive straight until we are 0.5m from the wall
      {
        triggered = MOVE_CheckSensor(sens);
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered)
        *movBack += (ir - temp); //Calculate dist required to move Back
    }
    
    if(FInNext && !triggered){
      resetIRPos(); ir = IR_Measure(); //Face the IR forward and get reading
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
      while((ir > 500) && !triggered)  //Drive straight until we are 0.5m from the wall
      {
        triggered = MOVE_CheckSensor(sens);
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered)
        *movBack += (ir - 500) + 900; //Calculate dist required to move Back
      
    } else if (!FInNext && !triggered){
      if(LHWallF && !triggered)
        triggered = IROBOT_WallFollow(DIR_CCW, sens, 600, movBack);
      else if(RHWallF && !triggered)
        triggered = IROBOT_WallFollow(DIR_CW, sens, 600, movBack);
    }
  }
  else  //Nothing to wall follow off
  {
    triggered = MOVE_Straight(BLIND_TOP_SPEED, blind_driveForwardDist, true, sens, movBack);
    if(FInNext && !triggered) //Wall in front for us to follow?
    {
      resetIRPos(); ir = IR_Measure(); //Face the IR forward and get reading
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
      while((ir > 500) && !triggered)  //Drive straight until we are 0.5m from the wall
      {
        triggered = MOVE_CheckSensor(sens);
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered)
        *movBack += (ir - 500) + blind_driveForwardDist; //Calculate dist required to move Back
    }
    else if(!FInNext && !triggered)
    {
      triggered = MOVE_Straight(BLIND_TOP_SPEED, blind_driveForwardDist, true, sens, movBack);
    }
  }
  
  return triggered;
}

/*! @brief Gets the flood fill value of the box in front of the robots 
 *         virtual position.
 *  
 *  @param currOrd - The current virtual position of the robot. 
 *  @return Flood fill value of the square in front of the robot
 */
int16_t getNextPathVal(TORDINATE currOrd){
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
  uint8_t lowestWall = 0; /* Indicates where the lowest wall was found */
  int16_t lowestSoFar = PATH_Path[currOrd.x][currOrd.y];
  TSENSORS sens;
  int16_t temp;

  //Check if wall not in front of us
  if(!PATH_GetMapInfo(currOrd, BOX_Front)){
    temp = getNextPathVal(currOrd);       //Get the flood fill value at that square
    if(temp < lowestSoFar && temp != -1){ //If its the lowest seen so far
      lowestWall = 0;     //Lowest wall at case 0 (in Front)
      lowestSoFar = temp;
    }
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Left)){
    PATH_UpdateOrient(1, DIR_CCW); //Virtually turn the robot (so its facing left)
    temp = getNextPathVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      lowestWall = 1;     //Lowest wall at case 1 (Left)
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(1, DIR_CW); //Virtually reset the robot
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Right)){
    PATH_UpdateOrient(1, DIR_CW); //Virtually turn the robot
    temp = getNextPathVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      lowestWall = 2;     //Lowest wall at case 2 (Right)
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(1, DIR_CCW); //Virtually reset the robot
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Back)){
    PATH_UpdateOrient(2, DIR_CCW); //Virtually turn the robot
    temp = getNextPathVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      lowestWall = 3;     //Lowest wall at case 3 (Back)
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(2, DIR_CW); //Virtually reset the robot
  }

  switch(lowestWall){
    case 0:
      //Wall in Front don't need to turn
      break;
    case 1:
      MOVE_Rotate(DRIVE_ROTATE_SPEED, (90 - ANGLE_ER), DIR_CCW, &sens); //Left
      PATH_UpdateOrient(1, DIR_CCW);
      break;
    case 2:
      MOVE_Rotate(DRIVE_ROTATE_SPEED, (90 - ANGLE_ER), DIR_CW, &sens); //Right
      PATH_UpdateOrient(1, DIR_CW);
      break;
    case 3:
      MOVE_Rotate(DRIVE_ROTATE_SPEED, 180, DIR_CCW, &sens); //Back
      PATH_UpdateOrient(2, DIR_CCW);
      break;
  }
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

/*! @brief Attempts to find a victim at the robots current position.
 *
 *  @return TRUE - If a victim was found
 */
bool victimFound(void){
  uint8_t rxdata;
  USART_OutChar(OP_SENSORS);  //Grab data about P17: Infrared data
  USART_OutChar(OP_SENS_IR);
  rxdata = USART_InChar();
  
  //return (rxdata == 250 || rxdata == 246 || rxdata == 254);
  LCD_PrintInt((int)rxdata, TOP_LEFT);
  return (rxdata == 250 || rxdata == 246 || rxdata == 254);
}

/*! @brief Determines if all victims have been found, if not - it will perform
 *  a scan at the robots current location.
 *
 *  @param curr - The current position of the robot
 *  @return TRUE - If both victims were found.
 */
bool areAllVictimsFound(TORDINATE curr){
  //Set initial victim locations to a unreasonable location, to indacte not found
  static TORDINATE vic1 = {255, 255};
  static TORDINATE vic2 = {255, 255};
  static bool bothVicsFound = false;

  if(!bothVicsFound){ //First of all, make sure that all victims havent already been found
    if(victimFound()){ //A victim was found
      if(vic1.x == 255){ //If victim 1 has yet to be found
        vic1.x = curr.x; vic1.y = curr.y; //Set vics location to our position
        playSong(0);
      } else {
        if(!(curr.x == vic1.x && curr.y == vic1.y)) //If the current location isnt victim 1
        {
          vic2.x = curr.x; vic2.y = curr.y; //Victim 2 was found!!
          playSong(1);
          bothVicsFound = true;
        }
      }
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
