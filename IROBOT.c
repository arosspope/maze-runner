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
#include "USART.h"
#include "MOVE.h"
#include "SM.h"
#include "LCD.h" //TODO: Remove references to LCD
#include "IROBOT.h"

#define OP_START  128
#define OP_FULL   132

//Optimal speeds for driving the iROBOT
#define DRIVE_TOP_SPEED   200
#define DRIVE_TURN_SPEED  150

/* Private function prototypes */
static void wallAlign(uint16_t wallLocation);
static uint16_t closestObject(void);
static void resetIRPos(void);
/* End Private function prototypes */

bool IROBOT_Init(void){
  return USART_Init() && SM_Init() && MOVE_Init();
}

void IROBOT_Start(void){
  //Put the IROBOT in full control mode
  USART_OutChar(OP_START);
  USART_OutChar(OP_FULL);
}

//TODO: Must remove - test routine, used for testing specific bits of code.
void IROBOT_Test(void){

}

void IROBOT_WallFollow(void){
  double tolerance, dist;    //dist variable stores the IR sensor reading

  //Reset IR position, find the closest wall (for left-wall follow) and align
  resetIRPos();
  wallAlign(closestObject());

  //Store current distance reading at a 45 degree angle to the wall from IR sensor
  tolerance = IR_Measure();  
  dist = tolerance;
  
  while (!MOVE_CheckSensor()){  //if no sensors have been triggered i.e. bumper
    if((dist < (tolerance + 5)) && (dist > (tolerance - 5))){  //while IR reading is between plus/minus 5mm execute following lines
      MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TOP_SPEED);          //left wheel and right wheel drive same speed.
    }
    else if (dist < (tolerance - 5)){      //Robot to close to wall
      MOVE_DirectDrive(DRIVE_TOP_SPEED, DRIVE_TURN_SPEED);
    }
    else{                                  //Robot to far away from wall
      MOVE_DirectDrive(DRIVE_TURN_SPEED, DRIVE_TOP_SPEED);
    }
    
    dist = IR_Measure();                   //Keep checking wall distance.
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
    MOVE_Rotate(moveAngle, DIR_CW); //Rotate robot so its parrallel with the wall

    //Calculate steps required to get IR sensor pointing 45 degs from 360 (CCW)
    if(moveAngle <= 45){
      SM_Move((uint16_t)((45 - moveAngle)/SM_F_STEP_RESOLUTION), DIR_CW);
    }else{ //Its greater than 45 degs
      SM_Move((uint16_t)((moveAngle - 45)/SM_F_STEP_RESOLUTION), DIR_CCW);
    }

  } 
  else if ((wallLocation <= 150)){
    moveAngle = (uint16_t)(270 - (wallLocation * SM_F_STEP_RESOLUTION));
    MOVE_Rotate(moveAngle, DIR_CCW);
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
  SM_Move(orientation, DIR_CCW);              //Move back those number of steps
}