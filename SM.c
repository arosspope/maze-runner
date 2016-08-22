/*! @file SM.c
 *
 *  @brief Stepper Motor routines.
 *
 *  This contains the functions for operating the Stepper Motor.
 *
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */
#include <pic.h>
#include <xc.h>
#include "LCD.h"
#include "SM.h"

/* Definition of required energising to achieve different 'steps' */
#define STEP0 0b00111100
#define STEP1 0b00101110
#define STEP2 0b00101011
#define STEP3 0b00100111
#define STEP4 0b00110101
#define STEP5 0b00010111
#define STEP6 0b00011011
#define STEP7 0b00011110
#define OFF   0b00111001

signed int STEP_COUNT = 0;

bool SM_Init(void) {
  TRISC = 0b00000000; //Set PORTC to all outputs
  PORTC = STEP0;      //Energise stepper motor to a known winding (STEP 0)

  return true;
}

void SM_Move(unsigned int steps, TDIRECTION dir) {
  static unsigned int currentStep = 0; //Init currentStep to 0 (STEP 0)

  //Update the total step count
  if (dir == DIR_CCW) {
    STEP_COUNT += steps; //Increment count for CCW rotation
  } else {
    STEP_COUNT -= steps; //Decrement for CW rotation
  }

  for (; steps != 0; --steps) {
    //Increment or decrement step depending on direction
    switch (dir) {
      case DIR_CCW: currentStep = (currentStep + 1) % 8;
        break;
      case DIR_CW: currentStep = (currentStep - 1) % 8;
        break;
    }

    switch (currentStep) {
      case 0: PORTC = STEP0; break;
      case 1: PORTC = STEP1; break;
      case 2: PORTC = STEP2; break;
      case 3: PORTC = STEP3; break;
      case 4: PORTC = STEP4; break;
      case 5: PORTC = STEP5; break;
      case 6: PORTC = STEP6; break;
      case 7: PORTC = STEP7; break;
      default:PORTC = OFF; break; //De-energise windings.
    }

    __delay_ms(10); //10ms delay between energising
    PORTC = OFF;
  }

  LCD_Print(STEP_COUNT, BM_LEFT); //Update the step count on the LCD
}