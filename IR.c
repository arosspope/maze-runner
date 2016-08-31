/*! @file IR.c
 *
 *  @brief Routines for the IR sensor.
 *
 *  This contains the functions for operating Infra-Red (IR) distance sensor.
 *
 *  @author Andrew.P, Andrew.T
 *  @date 02-08-2016
 */
#include <math.h>
#include "IR.h"
#include "LCD.h" //TODO: - Will need to be removed
#include "ADC.h"

bool IR_Init(void) {
  return ADC_Init();
}

double calcDistance(double ADCdata) {
  /* The following uses ranges of raw ADC data, to select the most 'accurate'
   * distance conversion equation.
   */
  double dist_cm; 
  
  if( ADCdata >= 398 && ADCdata < 516 ){  //20-30
    dist_cm = ((ADCdata-749)/-11.7); 
  } 
  else if ( ADCdata >= 304 && ADCdata < 398 ){ //30-40
    dist_cm = ((ADCdata-680)/-9.4);
  }
  else if ( ADCdata >= 244 && ADCdata < 304 ){
    dist_cm = ((ADCdata-544)/-6);
  }
  else if ( ADCdata >= 201 && ADCdata < 244 ){
    dist_cm = ((ADCdata-459)/-4.3);
  }
  else if ( ADCdata >= 174 && ADCdata < 201){
    dist_cm = ((ADCdata-363)/-2.7);
  }
  else if ( ADCdata >= 157 && ADCdata < 174){
    dist_cm = ((ADCdata-293)/-1.7);
  }
  else if ( ADCdata >= 137 && ADCdata < 157 ){
    dist_cm = ((ADCdata-317)/-2);
  }
  else if ( ADCdata >= 126 && ADCdata < 137){
    dist_cm = ((ADCdata-240.5)/-1.15);
  }
  else if ( ADCdata >= 113 && ADCdata < 126){
    dist_cm = ((ADCdata-250.5)/-1.25);
  }
  else if ( ADCdata >= 105 && ADCdata < 113 ){
    dist_cm = ((ADCdata - 201)/-0.8);
  }
  else if ( ADCdata >= 96 && ADCdata < 105 ){
    dist_cm = ((ADCdata-213)/-0.9);
  }
  else if ( ADCdata >= 89.33 && ADCdata < 96 ){
    dist_cm = ((ADCdata - 183.1)/-0.67);
  }
  else if ( ADCdata >= 84 && ADCdata < 89.33 ){
    dist_cm = ((ADCdata - 163.5)/-0.53);
  }
  
  return (dist_cm * 10); //Convert to mm before returning
}
  
double IR_Measure(void) {
  double data = 0;

  /* Get 20 samples from the ADC module and find the average */
  for (int i = 0; i < 20; i++) {
    data += ADC_GetVal();
  }
  data = (data / 20);
  
  LCD_Print((int) data, TOP_LEFT); //TODO: TEST CODE - Should be removed
  return calcDistance(data);  //Returns the converted ADC value to distance in mm
}
