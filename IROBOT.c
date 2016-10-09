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
#include "IR.h"
#include "EEPROM.h"
#include "USART.h"
#include "PATH.h"
#include "MOVE.h"
#include "SM.h"
#include "OPCODES.h"
#include "IROBOT.h"

/* Loading Song Notes to EEPROM (pre-loading only)
 * 
 * Note duration is stored next to the note:
 *    -e.g. DATA(D, 0.5s, C#, 0.2s); 'D' will play for 0.5seconds, 'C#' will play for 0.2s
 */
__EEPROM_DATA(72, 30, 76, 15, 78, 10, 81, 15);  //Song 0 - ADDR offset: 0x00 Simpsons Theme
__EEPROM_DATA(79, 20, 76, 15, 71, 15, 67, 15);
__EEPROM_DATA(72, 12, 84, 12, 57, 12, 69, 12);  //Song 1 - ADDR offset: 0x10 Mario underworld
__EEPROM_DATA(58, 12, 70, 12, 0, 6, 0, 6);
__EEPROM_DATA(88, 12, 88, 12, 88, 12, 84, 12);  //Song 2 - ADDR offset: 0x20 Mario 
__EEPROM_DATA(88, 12, 91, 12, 79, 12, 0, 6);
__EEPROM_DATA(1, 2, 3, 4, 5, 6, 7, 8);          //Song 3 - ADDR offset: 0x30 - Nothing
__EEPROM_DATA(9, 10, 11, 12, 13, 14, 15, 16);

//Optimal speeds for driving the iROBOT
#define DRIVE_TOP_SPEED   450
#define BLIND_TOP_SPEED   300
#define DRIVE_TURN_SPEED  330
#define DRIVE_ROTATE_SPEED 210
#define ANGLE_ER 3

/* Private function prototypes */
static void resetIRPos(void);
static void loadSongs(void);
static void playSong(uint8_t songNo);
static bool moveForwardFrom(TORDINATE ord, TSENSORS * sens, int16_t * movBack);
static bool findNextSquare(TORDINATE currOrd, bool doRotate);
static int16_t getNextPathVal(TORDINATE currOrd);
static bool areAllVictimsFound(TORDINATE curr);
static bool victimFound(void);
static bool wallFollow(TDIRECTION irDir, TSENSORS * sens, int16_t moveDist, int16_t * movBack);
static bool errorHandle(TORDINATE ord, TORDINATE wayP, TSENSORS sensor, int16_t movBack);
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

void IROBOT_MazeRun(void){
  bool bothVicsFound = false; TSENSORS sens;
  uint8_t i = 0; int16_t movBack = 0;
  
  /* Initialize way-points */
  TORDINATE home = {1, 3};
  TORDINATE currOrd = {1, 3};
  TORDINATE wayP1 = {2, 3};
  TORDINATE wayP2 = {3, 2};
  TORDINATE wayP3 = {3, 3};
  TORDINATE wayP4 = {3, 1};
  TORDINATE wayP5 = {0, 0};
  TORDINATE wayP6 = {2, 1};
  
  /* Initialize the list of way-points */
  TORDINATE wayList[7];
  wayList[0] = wayP1; wayList[1] = wayP2; wayList[2] = wayP3; wayList[3] = wayP4;
  wayList[4] = wayP5; wayList[5] = wayP6; wayList[6] = home;

  while(!bothVicsFound){
    //Loop through WayPoint List and continue to move around the maze, until both victims are found
    if(PATH_Plan(currOrd, wayList[i])) //If a path can be found
    {
      //While we haven't gotten to the selected way-point
      while(!(currOrd.x == wayList[i].x && currOrd.y == wayList[i].y))
      {
        //Check square for victims and determine if all have been found
        bothVicsFound = areAllVictimsFound(currOrd);
        if(bothVicsFound)
          break; //Break inner while loop and go home
        
        //Find next square to move to, and rotate robot to face
        findNextSquare(currOrd, true);                
        if(moveForwardFrom(currOrd, &sens, &movBack)) //If a Sensor was triggered during the move forward routine
        {
          //Handle the sensor
          if(!errorHandle(currOrd, wayList[i], sens, movBack))
            break; //If we cant calculate a path to the way-point; break and go to the next way-point
        } 
        else 
        {
          PATH_UpdateCoordinate(&currOrd); //Everything was fine, update position
        }
        movBack = 0;
      }
    } //If a path can't be found, move to the next way-point
    
    i = (i + 1) % 7; //Make sure to move within the way-point list
  }

  //We have found both victims, time to go home!
  movBack = 0;
  PATH_Plan(currOrd, home); //Plan the path back home
  while(!(currOrd.x == home.x && currOrd.y == home.y))
  {
    //Same functionality as before
    findNextSquare(currOrd, true);
    if(moveForwardFrom(currOrd, &sens, &movBack)){
      errorHandle(currOrd, home, sens, movBack);
    } else {
      PATH_UpdateCoordinate(&currOrd);
    }
    movBack = 0;
  }
  
  playSong(2); //Play a song when we have arrived
}

/*! @brief Handles scenarios where the bump or virtual wall sensor was triggered.
 *
 *  @param ord - The ordinate where the sensor was triggered.
 *  @param wayP - The way-point ordinate that the robot was trying to get too when triggered.
 *  @param movBack - Used for the bump sensor to tell the robot how far to move back
 * 
 *  @return TRUE - In most cases. Will return FALSE if a new path can't be calculated between
 *                 the robot and the way-point (path only re-calculated when encountering a virtual wall).
 */
static bool errorHandle(TORDINATE ord, TORDINATE wayP, TSENSORS sensor, int16_t movBack){
  bool rc = true;
  
  if(sensor.bump){
    MOVE_Straight(-180, movBack, false, 0, 0); //For bump sensor, only need to move back
  }
  
  if(sensor.wall){
    MOVE_Straight(-180, movBack, false, 0, 0); //For virtual wall, we need to move back and re-calculate path
    PATH_VirtWallFoundAt(ord);
    rc = PATH_Plan(ord, wayP);
  }
  
  return rc;
}

/*! @brief Perform a Wall-follow
 *
 *  @param irDir - Which way to face the direction of ir sensor (for left or right follow)
 *  @param sensor - A struct to hold information about sensors 
 *  @param moveDist - How far the robot should wall follow until it needs to stop (mm)
 *  @param movBack - A pointer to a variable that holds how far the robot moved before it was interrupted
 * 
 *  @return bool - TRUE if interrupted by a sensor
 */
static bool wallFollow(TDIRECTION irDir, TSENSORS * sens, int16_t moveDist, int16_t * movBack){
  double tolerance = 700; //Ensure we stay 700mm from the wall
  bool triggered = false; int16_t distmoved = 0; double dist;
  uint16_t orientation = SM_Move(0, DIR_CW);
 
  //Reset IR position then face IR sensor 45 degs in particular direction. If its
  //already at that position however, don't do anything
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
  
  dist = IR_Measure();  //Get current distance reading
  MOVE_GetDistMoved();  //Reset the distance moved encoders on the iRobot
  
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
    triggered = MOVE_CheckSensor(sens); //Check sensors
  }
  
  MOVE_DirectDrive(0,0);  //Stop iRobot
  
  if(triggered) //If robot was interrupted, calculate distance required to move back
    *movBack += distmoved;
  
  return triggered;
}

/*! @brief Will determine the best way to move forward into the next square, 
 *         from its current position. 
 *
 *  @param ord - The x/y coordinate to move forward from
 *  @param sensor - A struct to hold information about sensors 
 *  @param moveDist - How far the robot should wall follow until it needs to stop (mm)
 * 
 *  @note Assumes the robot has already been rotated to move into the next 
 *        grid location.
 */
static bool moveForwardFrom(TORDINATE ord, TSENSORS * sens, int16_t * movBack){
  bool triggered = false; double ir; int temp; int dist = 0;
  bool LWallF, RWallF, LHWallF, RHWallF, FInNext, BWall;
  TORDINATE nextOrd = ord;
  
  //Calculates the next grid position based on what way the robot is facing
  PATH_UpdateCoordinate(&nextOrd);
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
      if(LWallF)  //First do a left/right wall follow for 500mm
        triggered = wallFollow(DIR_CCW, sens, 500, movBack);
      else
        triggered = wallFollow(DIR_CW, sens, 500, movBack);
      
      if(!triggered) //If not triggered, do a front wall follow until within 500mm
      {
        resetIRPos(); ir = IR_Measure(); dist = 0; //Face the IR forward and get reading
        
        MOVE_GetDistMoved(); MOVE_DirectDrive(BLIND_TOP_SPEED, BLIND_TOP_SPEED);
        while((ir > 500) && !triggered)  //Drive straight until we are 500mm from the wall
        {
          triggered = MOVE_CheckSensor(sens);
          dist += MOVE_GetDistMoved();
          ir = IR_Measure();
        }
        MOVE_DirectDrive(0,0); //Stop the robot
        
        if(triggered)
          *movBack += dist; //Calculate distance required to move Back if triggered
      }
    }
    else if(BWall && !triggered){ //Do a Back-wall follow
      resetIRPos(); SM_Move(100, DIR_CW); ir = IR_Measure(); //Face the IR backward and get reading
      temp = (int16_t)ir; dist = 0; MOVE_GetDistMoved();
      
      MOVE_DirectDrive(BLIND_TOP_SPEED, BLIND_TOP_SPEED);
      while((ir < 900) && !triggered)  //Drive straight until we are 900mm away from the wall
      {
        triggered = MOVE_CheckSensor(sens);
        dist += MOVE_GetDistMoved();
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered)
        *movBack += dist;
      
      //Do a wall follow the rest of the way (600mm)
      if(LWallF && !triggered)
        triggered = wallFollow(DIR_CCW, sens, 600, movBack);
      else if(RWallF && !triggered)
        triggered = wallFollow(DIR_CW, sens, 600, movBack);
    }
    else if(!BWall && !FInNext && !triggered){
      //If no back-wall or front-wall - then wall follow for the full 1m
      if(LWallF)
        triggered = wallFollow(DIR_CCW, sens, 1000, movBack);
      else
        triggered = wallFollow(DIR_CW, sens, 1000, movBack);
    }
  } 
  else if(LHWallF || RHWallF) //If we can follow a wall for half-the way
  {
    //If there's not a wall to the left/right of us in this box, but there is one in the next
    if(BWall && !triggered){ //If we can back wall follow
      resetIRPos(); SM_Move(100, DIR_CW); ir = IR_Measure();
      temp = (int16_t)ir; dist = 0; MOVE_GetDistMoved();
      
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED);
      while((ir < 900) && !triggered)
      {
        triggered = MOVE_CheckSensor(sens);
        dist += MOVE_GetDistMoved();
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0);
      
      if(triggered)
        *movBack += dist;
    }
    dist = 0;
    
    //If we can also front-wall follow
    if(FInNext && !triggered){
      resetIRPos(); ir = IR_Measure();
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED); dist = 0; MOVE_GetDistMoved();
      while((ir > 500) && !triggered)
      {
        triggered = MOVE_CheckSensor(sens);
        dist += MOVE_GetDistMoved();
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered){
        *movBack += dist;
        if(BWall)
          *movBack += 400;
      }
    } else if (!FInNext && !triggered){
      //Wall-follow the rest of the way
      if(LHWallF && !triggered)
        triggered = wallFollow(DIR_CCW, sens, 600, movBack);
      else if(RHWallF && !triggered)
        triggered = wallFollow(DIR_CW, sens, 600, movBack);
    }
  }
  else  //Nothing to wall follow off
  {
    triggered = MOVE_Straight(BLIND_TOP_SPEED, 500, true, sens, movBack);
    if(FInNext && !triggered) //Wall in front for us to follow?
    {
      resetIRPos(); ir = IR_Measure(); dist = 0;//Face the IR forward and get reading
      MOVE_DirectDrive(BLIND_TOP_SPEED,BLIND_TOP_SPEED); MOVE_GetDistMoved();
      while((ir > 500) && !triggered)  //Drive straight until we are 0.5m from the wall
      {
        triggered = MOVE_CheckSensor(sens);
        dist += MOVE_GetDistMoved();
        ir = IR_Measure();
      }
      MOVE_DirectDrive(0,0); //Stop the robot
      
      if(triggered)
        *movBack += dist; //Calculate dist required to move Back
    }
    else if(!FInNext && !triggered)
    {
      triggered = MOVE_Straight(BLIND_TOP_SPEED, 500, true, sens, movBack);
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
static int16_t getNextPathVal(TORDINATE currOrd){
  PATH_UpdateCoordinate(&currOrd); //Virtually move the robot forward
  return PATH_Path[currOrd.x][currOrd.y]; //Get flood-fill value at next square
}

/*! @brief Finds the next square to move into (based on flood fill) and orient
 *         the robot to face that square.
 * 
 *  @param currOrd - The box to move from
 *  @param doRotate - Indiactes whether or not we want the robot to rotate to face the next lowest sqaure
 *  @return TRUE - If it could find a lower square on the path
 */
static bool findNextSquare(TORDINATE currOrd, bool doRotate){
  uint8_t lowestWall = 0; /* Indicates where the lowest wall was found */
  int16_t lowestSoFar = PATH_Path[currOrd.x][currOrd.y];
  TSENSORS sens;
  int16_t temp;

  //Check if wall not in front of us
  if(!PATH_GetMapInfo(currOrd, BOX_Front)){
    temp = getNextPathVal(currOrd);       //Get the flood fill value at that square
    if(temp < lowestSoFar && temp != -1){ //If its the lowest seen so far
      lowestWall = 1;     //Lowest wall at case 1 (in Front)
      lowestSoFar = temp;
    }
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Left)){
    PATH_UpdateOrient(1, DIR_CCW); //Virtually turn the robot (so its facing left)
    temp = getNextPathVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      lowestWall = 2;     //Lowest wall at case 2 (Left)
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(1, DIR_CW); //Virtually reset the robot
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Right)){
    PATH_UpdateOrient(1, DIR_CW); //Virtually turn the robot
    temp = getNextPathVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      lowestWall = 3;     //Lowest wall at case 3 (Right)
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(1, DIR_CCW); //Virtually reset the robot
  }
  
  if(!PATH_GetMapInfo(currOrd, BOX_Back)){
    PATH_UpdateOrient(2, DIR_CCW); //Virtually turn the robot
    temp = getNextPathVal(currOrd);
    if(temp < lowestSoFar && temp != -1){
      lowestWall = 4;     //Lowest wall at case 4 (Back)
      lowestSoFar = temp;
    }
    PATH_UpdateOrient(2, DIR_CW); //Virtually reset the robot
  }

  if(doRotate){
    switch(lowestWall){
      case 1:
        //Wall in Front don't need to turn
        break;
      case 2:
        MOVE_Rotate(DRIVE_ROTATE_SPEED, (90 - ANGLE_ER), DIR_CCW, &sens); //Left
        PATH_UpdateOrient(1, DIR_CCW);
        break;
      case 3:
        MOVE_Rotate(DRIVE_ROTATE_SPEED, (90 - ANGLE_ER), DIR_CW, &sens); //Right
        PATH_UpdateOrient(1, DIR_CW);
        break;
      case 4:
        MOVE_Rotate(DRIVE_ROTATE_SPEED, 180, DIR_CCW, &sens); //Back
        PATH_UpdateOrient(2, DIR_CCW);
        break;
      default:
        //We did not find a lower wall - the path we are on has failed
        break;
    }
  }

  return (lowestWall > 0);
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

/*! @brief Attempts to find a victim at the robots current position.
 *
 *  @return TRUE - If a victim was found
 */
static bool victimFound(void){
  uint8_t rxdata;

  /* Keep getting data about Packet 17 (Infared Byte), until two consecutive
   * reads (15ms apart) return the same value.
   */
  do {
    USART_OutChar(OP_SENSORS);
    USART_OutChar(OP_SENS_IR);
    rxdata = USART_InChar();
    __delay_ms(15);
    USART_OutChar(OP_SENSORS);
    USART_OutChar(OP_SENS_IR);
  } while(rxdata != USART_InChar());

  //Infrared ranges we are checking: Red-Buoy + ForceField, Green-Buoy + FF, RB + GB + FF, GB, RB
  return (rxdata == 250 || rxdata == 246 || rxdata == 254 || rxdata == 244 || rxdata == 248 );
}

/*! @brief Determines if all victims have been found, if not - it will perform
 *  a scan at the robots current location.
 *
 *  @param curr - The current position of the robot
 *  @return TRUE - If both victims were found.
 */
static bool areAllVictimsFound(TORDINATE curr){
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