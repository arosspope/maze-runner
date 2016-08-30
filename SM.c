/*! @file SM.c
 *
 *  @brief Stepper Motor routines.
 *
 *  This contains the functions for operating the Stepper Motor.
 *
 *  @author Andrew.P 
 *  @date 02-08-2016
 */
#include "SM.h"
#include "SPI.h"

/* Masks to construct control byte for the SPI module */
#define CLK_PIC_MASK 0b00001000
#define ENABLE_MASK  0b00000001
#define H_STEP_MASK  0b00000100 //Half-step
#define F_STEP_MASK  0b00000000 //Full-step

bool SM_Init(void) {
  return true; //TODO: Determine if SM is the only module to use SPI, if so - initialise here
}

int SM_Move(unsigned int steps, TDIRECTION dir) {
  static int stepCount = 0;
  uint8_t controlByte = 0;

  //Select the stepper motor module via SPI
  SPI_SelectMode(SPI_SM);

  //Enable and Construct the control byte for the SPI module and send
  controlByte = (ENABLE_MASK | CLK_PIC_MASK | H_STEP_MASK | dir);
  SPI_SendData(controlByte);


  //Update the total step count
  if (dir == DIR_CCW) {
    stepCount += steps; //Increment count for CCW rotation
  } else {
    stepCount -= steps; //Decrement for CW rotation
  }
  
  for (; steps != 0; --steps){
    //Pulse the Stepper motor the desired amount of steps
    RC2 = 1; NOP(); RC2 = 0;
    __delay_ms(10);
  }
  
  SPI_SendData(0); //Disable the SM module

  return stepCount;
}