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

#define OP_START      128
#define OP_FULL       132
#define OP_DEMO       136
#define OP_DEMO_FIG8  4

bool IROBOT_Init(void){
  bool isSuccess = USART_Init() && SM_Init();

  //If initialisation was successfull, send the startup opcodes to the IROBOT
//  if (isSuccess){
//    USART_OutChar(OP_START);
//    USART_OutChar(OP_FULL);
//  }
  return isSuccess;
}

void IROBOT_Start(void){
  USART_OutChar(OP_START);
  USART_OutChar(OP_FULL);
}

void IROBOT_Scan360(void){
  uint8_t orientation, steps, closestObject;
  double smallestIR = 4000.0; //Set this initial value to outside the range of the IR sensor
  double data;

  //Move the Stepper motor 0 steps to obtain the current orientation
  orientation = SM_Move(0, DIR_CW);

  //Move stepper motor 360 degs, taking IR samples each time
  for(uint8_t i = 0; i < (SM_H_STEPS_FOR_180*2); i++){
    data = IR_Measure();            //Measure the distance
    
    if (data < smallestIR)
    {
      smallestIR = data;            //Update the data to smallest reading
      closestObject = orientation;  //Update the step at which the closest object was found
    }

    orientation = SM_Move(1, DIR_CW);   //Move the motor half a step CW
    LCD_Print((int) data, TOP_RIGHT);  //Print the IR reading to LCD
  }

  //Calculate the amount of steps required to point sensor back to the cloest object
  steps = ((SM_H_STEPS_FOR_180*2) - 1 ) - closestObject;
  SM_Move(steps, DIR_CCW);
}

void IROBOT_Test(void){
  //For testing purposes only - send test opcodes to the irobot
  //Figure-8 test
  USART_OutChar(OP_DEMO);
  USART_OutChar(OP_DEMO_FIG8);
}