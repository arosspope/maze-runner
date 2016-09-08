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

/*! @brief Determines the distance from a raw ADC value.
 *
 *  @param ADCdata - The raw ADC data to convert
 *  @return distance - The distance in mm
 */
double calcDistance(double ADCdata) {
  /* The following uses ranges of raw ADC data, to select the most 'accurate'
   * distance conversion equation.
   */
  double dist_cm = 0;

  //Any ADC value greater than 510 (i.e. the dead-zone) will read as 0mm)
  if( ADCdata >= 379 && ADCdata < 510 ){  //20-30
    dist_cm = ((ADCdata-772)/-13.1); 
  } 
  else if ( ADCdata >= 295 && ADCdata < 379 ){ //30-40
    dist_cm = ((ADCdata-631)/-8.4);
  }
  else if ( ADCdata >= 240 && ADCdata < 295 ){  //40-50
    dist_cm = ((ADCdata-515)/-5.5);
  }
  else if ( ADCdata >= 197 && ADCdata < 240 ){  //50-60
    dist_cm = ((ADCdata-455)/-4.3);
  }
  else if ( ADCdata >= 173 && ADCdata < 197){  //60-70
    dist_cm = ((ADCdata-341)/-2.4);
  }
  else if ( ADCdata >= 153 && ADCdata < 173){  //70-80
    dist_cm = ((ADCdata-313)/-2);
  }
  else if ( ADCdata >= 137 && ADCdata < 153 ){  //80-90
    dist_cm = ((ADCdata-281)/-1.6);
  }
  else if ( ADCdata >= 122 && ADCdata < 137){  //90-100
    dist_cm = ((ADCdata-272)/-1.5);
  }
  else if ( ADCdata >= 108 && ADCdata < 122){  //100-110
    dist_cm = ((ADCdata-262)/-1.4);
  }
  else if ( ADCdata >= 99 && ADCdata < 108 ){ //110-120
    dist_cm = ((ADCdata - 207)/-0.9);
  }
  else if ( ADCdata >= 91 && ADCdata < 99 ){  //120-130
    dist_cm = ((ADCdata-195)/-0.8);           //the equation between 120-140 is the same
  }
  else if ( ADCdata >= 83 && ADCdata < 91 ){  //130-140
    dist_cm = ((ADCdata - 195)/-0.8);
  }
  else if ( ADCdata >= 78 && ADCdata < 83 ){  //140-150
    dist_cm = ((ADCdata - 152)/-0.5);
  }
  else if (ADCdata >= 0 && ADCdata < 84){
    dist_cm = 150;
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
