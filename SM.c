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

/* Masks to construct control byte for SM control within the SPI module */
#define CLK_PIC_MASK 0b00001000
#define ENABLE_MASK  0b00000001
#define H_STEP_MASK  0b00000100 //Half-step
#define F_STEP_MASK  0b00000000 //Full-step

const uint8_t SM_F_STEPS_FOR_180 = 100;  //200 Full steps for 180 deg movement
const double SM_F_STEP_RESOLUTION = 1.8; //0.9 degrees per half step

bool SM_Init(void) {
  return SPI_Init(); //Return initialisation of the SPI module
}

/*! @brief Calculates the step orientation in relation to a 360 deg circle.
 *
 *  @param a - The step value to convert
 *  @return step - The normalised step orientation within the 360 deg circle.
 */
uint16_t calcOrientation(int a)
{
  int b = (SM_F_STEPS_FOR_180 * 2); //Get the amount of steps for 360 degs

  int step = a % b;
  if (step < 0){
    step += b;
  }
  
  return step;
}

uint16_t SM_Move(unsigned int steps, TDIRECTION dir) {
  static uint16_t orientation = 0; //Initially, we assume the stepper motor is at pos 0
  uint8_t controlByte = 0;

  //Select the stepper motor module via SPI
  SPI_SelectMode(SPI_SM);

  //Enable and Construct the control byte for the SPI module and send
  controlByte = (ENABLE_MASK | CLK_PIC_MASK | F_STEP_MASK | dir);
  SPI_SendData(controlByte);


  //Update the step orientation
  if (dir == DIR_CW) {
    orientation = calcOrientation((orientation += steps)); //Increment orientation for CW rotation
  } else {
    orientation = calcOrientation((orientation -= steps));
  }
  
  for (; steps != 0; --steps){
    //Pulse the Stepper motor the desired amount of steps
    RC2 = 1; NOP(); RC2 = 0;
    __delay_ms(10);
  }
  
  SPI_SendData(0);          //Disable the SM module
  SPI_SelectMode(SPI_NONE); //Set SPI to reference no module

  return orientation;
}