/*! @file LED.c
 *
 *  @brief LED routines for the PIC16F87XA.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#include "LED.h"

bool LED_Init(void) {
  //Set the relevant PORTB pins to output mode
  TRISBbits.TRISB0 = 0;
  TRISBbits.TRISB1 = 0;

  //Set All LEDs to off initially
  LED_0 = 1;
  LED_1 = 1;

  return true;
}