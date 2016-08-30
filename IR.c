/*! @file IR.c
 *
 *  @brief Routines for the IR sensor.
 *
 *  This contains the functions for operating Infra-Red (IR) distance sensor.
 *
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */
#include <math.h>
#include "IR.h"
#include "LCD.h"
#include "ADC.h"

#define IR_CONSTANT 1.07
#define DIST_EQ(x) (0.000009*x - 0.00002)

bool IR_Init(void) {
  return ADC_Init();
}

double calcDistance(double ADCdata) {
  //Using the distance equation, we must find the Inverse distance * IR_CONSTANT
  double distance = (1 / DIST_EQ(ADCdata)) * IR_CONSTANT;
  return round(distance);
}

double IR_Measure(void) {
  double data = 0;

  /* Get 20 samples from the ADC module and find the average */
  for (int i = 0; i < 20; i++) {
    data += ADC_GetVal();
  }
  data = (data / 20);

  return calcDistance(data);  //Returns the converted ADC value to distance in mm
}
