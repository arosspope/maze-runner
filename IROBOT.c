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
#include "IROBOT.h"
#include "IR.h"
#include "USART.h"
#include "SM.h"
#include "LCD.h"

#define OP_START      128
#define OP_FULL       132
#define OP_DEMO       136
#define OP_DEMO_FIG8  4
#define OP_DRIVE      137
#define OP_DRIVE_DIRECT 145
#define OP_SENSORS    142
#define OP_SENS_DIST  19  /* Distance travelled since last call */
#define OP_SENS_ANGLE 20  /* Angle turned since last call */
#define OP_SENS_GROUP 1   /* Will return information about bump, wall, cliff, and virtual wall sensors */

/* Private function prototypes */
bool rotateRobot(uint16_t angle, TDIRECTION dir);
bool sensorTriggered(void);
void drive(int16_t leftWheelVel, int16_t rightWheelVel);
void wallAlign(void);
/* End Private function prototypes */

bool IROBOT_Init(void){
  return USART_Init() && SM_Init();
}

void IROBOT_Start(void){
  //Put the IROBOT in full control mode
  USART_OutChar(OP_START);
  USART_OutChar(OP_FULL);
}

void IROBOT_Scan360(void){
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
    LCD_PrintInt((int) data, TOP_RIGHT);  //Print the IR reading to LCD
  }

  //Calculate the amount of steps required to point sensor back to the closest object
  stepsBack = ((stepsFor360 - 1) + offset - closestObject) % stepsFor360;
  SM_Move(stepsBack, DIR_CCW);
}

bool IROBOT_DriveStraight(int16_t dist){
  uint16union_t rxdata;
  int16_t distanceTravelled = 0;
  bool sensorTrig = false;
  
  //Get data from the Irobot regarding distance to reset the distance travelled
  USART_OutChar(OP_SENSORS);
  USART_OutChar(OP_SENS_DIST);
  rxdata.s.Hi = USART_InChar(); //Dummy read of sensor values
  rxdata.s.Lo = USART_InChar();
  
  drive(200, 200); //Tell the IROBOT to drive straight at 200 mm/s
  
  //Let the robot drive until it reaches the desired distance
  while((distanceTravelled < dist) && !sensorTrig){
    //Get distance traveled since last call
    USART_OutChar(OP_SENSORS);
    USART_OutChar(OP_SENS_DIST);
    rxdata.s.Hi = USART_InChar();
    rxdata.s.Lo = USART_InChar();
    
    distanceTravelled += (int16_t) rxdata.l;
    LCD_PrintInt(distanceTravelled, BM_LEFT);
    LCD_PrintInt((int)IR_Measure(), TOP_RIGHT);
    sensorTrig = sensorTriggered();
  }
  
  drive(0, 0); //Tell the IROBOT to stop moving
  
  return sensorTrig;
}

void IROBOT_DriveSquare(void){
  uint8_t i = 0;
  bool triggered = false;
  
  while((i < 4) && !triggered){
    triggered |= IROBOT_DriveStraight(1000); //Drive straight for 1m
    triggered |= rotateRobot(90, DIR_CCW);   //Rotate 90 degs CCW
    __delay_ms(1000);                        //Forcing a delay makes the robot turn more accurately
    i++;
  }
}

void IROBOT_WallFollow(void){
  double tolerance, dist;    //dist variable stores the IR sensor reading
  uint16_t orientation;
  
  //Reset IR sensor to pos 0
  orientation = SM_Move(0, DIR_CW);
  SM_Move(orientation, DIR_CCW);
  
  //Find the position of the closest wall (should be left hand side of robot) and align
  IROBOT_Scan360();
  wallAlign();
  
  tolerance = IR_Measure();  //store current distance reading at a 45 degree angle to the wall from IR sensor 
  dist = tolerance;
  
  while (!sensorTriggered()){  //if no sensors have been triggered i.e. bumper
    if((dist < (tolerance + 5)) && (dist > (tolerance - 5))){  //while IR reading is between plus/minus 5mm execute following lines
      drive(200, 200);          //left wheel and right wheel drive same speed.
    }
    else if (dist < (tolerance - 5)){  //if robot is closer to the wall than the specified distance.
      drive(200, 150);            //slow down the right wheel.
    }
    else{  //if the robot is further away from the wall than specified distance. 
      drive (150, 200);           //slow down the left wheel. 
    }
    
    dist = IR_Measure();          //keep checking wall distance.
    
    LCD_PrintInt((int)dist, TOP_RIGHT); //Print to IR to screen
  }
  
  drive (0,0);  //if sensor is triggered, stop the IROBOT
}

/* @brief Will rotate the robot and align the robot parallel to the closest wall. 
 * 
 */
void wallAlign(void){
  uint16_t orientation, moveAngle;
  
  orientation = SM_Move(0, DIR_CW); //Move SM 0 steps to get its current orientation

  if(orientation >= 150){ //greater than 270 degrees (from 0 CW)
    moveAngle = (uint16_t) ((orientation * SM_F_STEP_RESOLUTION) - 270);
    rotateRobot(moveAngle, DIR_CW); //Rotate robot so its parrallel with the wall

    //Calculate steps required to get IR sensor pointing 45 degs from 360 (CCW)
    if(moveAngle <= 45){
      SM_Move((uint16_t)((45 - moveAngle)/SM_F_STEP_RESOLUTION), DIR_CW);
    }else{ //Its greater than 45 degs
      SM_Move((uint16_t)((moveAngle - 45)/SM_F_STEP_RESOLUTION), DIR_CCW);
    }

  } 
  else if ((orientation <= 150)){ //&& (orientation >= 100)) { //Less than 270 degs
    moveAngle = (uint16_t)(270 - (orientation * SM_F_STEP_RESOLUTION));
    rotateRobot(moveAngle, DIR_CCW);
    SM_Move((uint16_t)((moveAngle + 45) / SM_F_STEP_RESOLUTION), DIR_CW);
  }
}

/* @brief Rotates the robot to a particular orientation (angle within a circle).
 *
 * @param angle - Angle to rotate through in specified direction.
 * @param dir - The direction to rotate
 * 
 * @return bool - True if the rotate was interrupted due to sensor triggering
 */
bool rotateRobot(uint16_t angle, TDIRECTION dir){
  uint16union_t rxdata;
  int16_t angleMoved = 0;
  
  bool sensorTrig = false;
  
  //Get current angle moved to reset the angle moved count
  USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_ANGLE);
  rxdata.s.Hi = USART_InChar(); //Dummy read of both bytes to clear the receive buffer
  rxdata.s.Lo = USART_InChar(); 

  if (dir == DIR_CCW){
    drive(-210, 210); //Make the robot turn CCW @ 210mm/s
  } else {
    drive(210, -210); //Make the robot turn CW @ 210mm/s
  }
  
  while ((angleMoved < angle) && !sensorTrig)
  {
    //Get Angle since last movement
    USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_ANGLE);
    rxdata.s.Hi = USART_InChar();
    rxdata.s.Lo = USART_InChar();

    if(dir == DIR_CCW){
      angleMoved += (int16_t) rxdata.l; //CCW direction returns positive angles
    }else{
      angleMoved += ((int16_t) rxdata.l * -1); //CW direction returns negative angles
    }
    
    sensorTrig = sensorTriggered();
  }
  
  drive(0, 0); //Tell the IROBOT to stop rotating
  return sensorTrig;
}

/* @brief Determines if any of the relevant sensors have been triggered.
 *
 * @return TRUE - if one of the sensors have been tripped
 */
bool sensorTriggered(void){
  uint8_t data;
  bool sensorTriggered;

  //Tell the Robot to send back information regarding a group of sensors (will send 10 packets)
  USART_OutChar(OP_SENSORS);
  USART_OutChar(OP_SENS_GROUP);

  //1. Packet ID: 7 (Bump and Wheel drop)
  data = USART_InChar();
  sensorTriggered = (data & 0b00000011);   //We only care about the bump data so AND with mask
  
  //2. Packet ID: 8 (Wall)
  data = USART_InChar();  //Do dummy read as we don't care about this sensor

  //3. Packet ID: 9 (Cliff Left)
  sensorTriggered |= USART_InChar();

  //4. Packet ID: 10 (Cliff Front Left)
  sensorTriggered |= USART_InChar();

  //5. Packet ID: 11 (Cliff Front Right)
  sensorTriggered |= USART_InChar();

  //6. Packet ID: 12 (Cliff Right)
  sensorTriggered |= USART_InChar();

  //7. Packet ID: 13 (Virtual Wall)
  data = USART_InChar();  //Do dummy read as we don't care about this sensor

  //8. Packet ID: 14 (Low Side Driver and Wheel overcurrents)
  data = USART_InChar();  //Do dummy read as we don't care about this sensor

  //9/10. Packet ID: 15-16 (Unused)
  data = USART_InChar();  //Do dummy read as we don't care about these sensors
  data = USART_InChar();
  
  return sensorTriggered;
}

/* @brief Will drive the iRobot, by setting the left and right wheels to different velocities.
 *
 * @param leftWheelVel - The velocity of the left side wheel
 * @param rightWheelVel - The velocity of the right side wheel
 */
void drive(int16_t leftWheelVel, int16_t rightWheelVel){
  int16union_t rightBytes, leftBytes;

  rightBytes.l = rightWheelVel;
  leftBytes.l = leftWheelVel;

  USART_OutChar(OP_DRIVE_DIRECT);
  USART_OutChar(rightBytes.s.Hi); //Send the velocity for the right wheel
  USART_OutChar(rightBytes.s.Lo);
  USART_OutChar(leftBytes.s.Hi); //Send the velocity for the left wheel
  USART_OutChar(leftBytes.s.Lo);
}
