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
  
  //double distance = (1 / DIST_EQ(ADCdata)) * IR_CONSTANT;
  //return round(distance);
  double dist_mm; 
  
  if(ADCdata>398 && ADCdata<515){
    dist_mm= ((ADCdata -749)/-11.7); 
  } 
  else if (ADCdata>304 && ADCdata<398){ 
    dist_mm = ((ADCdata - 680/-9.4));
  }
  else if (ADCdata>244 && ADCdata<304){
    dist_mm = dist_mm = ((ADCdata-544)/-6);
  }
  else if (ADCdata>201 && ADCdata<244){
    dist_mm = ((ADCdata-459)/-4.3);
  }
  else if (ADCdata>174 && ADCdata <201){
    dist_mm = ((ADCdata - 363)/-2.7);
  }
  else if (ADCdata>157 && ADCdata<174){
    dist_mm = ((ADCdata-293)/-1.7);
  }
  else if (ADCdata>137 && ADCdata<157){
    dist_mm = ((ADCdata-317)/-2);
  }
  else if (ADCdata>126 && ADCdata<137){
    dist_mm = ((ADCdata-240.5)/-1.15);
  }
  else if (ADCdata>113&&ADCdata<126){
    dist_mm = ((ADCdata-250.5)/-1.25);
  }
  else if (ADCdata>105 && ADCdata<113){
    dist_mm = ((ADCdata - 201)/-0.8);
  }
  else if (ADCdata>96 && ADCdata<105){
    dist_mm = ((ADCdata-213)/-0.9);
  }
  else if (ADCdata>89.33 && ADCdata<96){
    dist_mm = ((ADCdata - 183.1)/-0.67);
  }
  else if (ADCdata>84 && ADCdata<89.33){
    dist_mm = ((ADCdata - 163.5)/-0.53);
  }
  
  return dist_mm;
}
  


double IR_Measure(void) {
  double data = 0;

  /* Get 20 samples from the ADC module and find the average */
  for (int i = 0; i < 20; i++) {
    data += ADC_GetVal();
  }
  data = (data / 20);
  
  LCD_Print((int) data, TOP_LEFT);
  return calcDistance(data);  //Returns the converted ADC value to distance in mm
}
