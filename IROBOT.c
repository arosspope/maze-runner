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
#include "USART.h"
#include "SM.h"
#include "LCD.h"

bool IROBOT_Init(void){
  //Send startup opcodes
  
  return true;
}

void IROBOT_Scan360(void){
  //Step 360 degrees and take a sample each time, and print to LCD screen
  //At completion point Sensor at closest object
}
