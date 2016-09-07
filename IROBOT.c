/*! @file IROBOT.c
 *
 *  @brief Module to control the iRobot.
 *
 *  This contains the functions for communicating with the iRobot through the
 *  create open interface.
 *
 *  @author
 *  @date 02-09-2016
 */
#include "IROBOT.h"
#include "IR.h"
#include "USART.h"
#include "SM.h"
#include "LCD.h"
#include "Drive.h"

#define OP_START      128
#define OP_FULL       132
#define OP_DEMO       136
#define OP_DEMO_FIG8  4
#define OP_SENSORS    142
#define OP_DRIVE      137
#define OP_SENS_DIST  19  /* Distance travelled since last call */
#define OP_SENS_ANGLE 20  /* Angle turned since last call */
#define OP_SENS_GROUP 1   /* Will return information about bump, wall, cliff, and virtual wall sensors */

/* Private function prototypes */
void orientateRobot(uint8_t orientation);
bool sensorTriggered(void);

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
    LCD_Print((int) data, TOP_RIGHT);  //Print the IR reading to LCD
  }

  //Calculate the amount of steps required to point sensor back to the closest object
  stepsBack = ((stepsFor360 - 1) + offset - closestObject) % stepsFor360;

  SM_Move(stepsBack, DIR_CCW);
  LCD_Print((int) smallestIR, BM_LEFT); //TEST-CODE
  LCD_Print((int) closestObject, BM_RIGHT); //TEST-CODE
  //TODO: TEST CODE - attempt to orient the robot with the object
  //orientateRobot(orientation);
}

void IROBOT_Test(void){
  //For testing purposes only - send test opcodes to the irobot - TODO: Remove
  //Figure-8 test
  USART_OutChar(OP_DEMO);
  USART_OutChar(OP_DEMO_FIG8);
  __delay_ms(2000);  //Let the figure-8 demo play for 10 seconds before returning to full mode
  __delay_ms(2000);
  __delay_ms(2000);
  __delay_ms(2000);
  __delay_ms(2000);
  USART_OutChar(OP_FULL);
}

void IROBOT_DriveStraight(void){
  driveStraight(500, 1000);  //TODO: test code
}

/* @brief Rotates the robot to a particular orientation (angle within a circle).
 *
 * @param orientation - A step value which corresponds to an angle.
 * @return void
 */
void orientateRobot(uint8_t orientation){
  int16union_t rxdata;
  int16_t angleMoved = 0;
  int16_t angleDesired = (int16_t)(orientation * SM_F_STEP_RESOLUTION); //Convert steps to degrees
  
  //Get current angle moved to reset the angle moved count
  USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_ANGLE);
  USART_InChar(); USART_InChar(); //Dummy read of both bytes to clear the receive buffer
  
  //Initiate a drive command @50mm/s and make the robot turn on the spot CW (0xFFFF)
  drive(50, 0xFFFF, 0);
  
  while (angleMoved < angleDesired)
  {
    //Get Angle since last movement
    USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_ANGLE);
    rxdata.s.Hi = USART_InChar();
    rxdata.s.Lo = USART_InChar();
    
    angleMoved += rxdata.l; //Add the angle moved since the last call to the total count 
  }
  drive(0, 0xFFFF, 0); //Tell the IROBOT to stop rotating
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
  sensorTriggered &= USART_InChar();

  //4. Packet ID: 10 (Cliff Front Left)
  sensorTriggered &= USART_InChar();

  //5. Packet ID: 11 (Cliff Front Right)
  sensorTriggered &= USART_InChar();

  //6. Packet ID: 12 (Cliff Right)
  sensorTriggered &= USART_InChar();

  //7. Packet ID: 13 (Virtual Wall)
  data = USART_InChar();  //Do dummy read as we don't care about this sensor

  //8. Packet ID: 14 (Low Side Driver and Wheel overcurrents)
  data = USART_InChar();  //Do dummy read as we don't care about this sensor

  //9/10. Packet ID: 15-16 (Unused)
  data = USART_InChar();  //Do dummy read as we don't care about these sensors
  data = USART_InChar();

  return sensorTriggered;
}