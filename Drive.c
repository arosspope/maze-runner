/*  @file Drive.c
 *
 *  drive speed, distance, direction
 *
 *
 *  @author Kate Leone (sn: 12011843)
 *  @date 04-09-2016
 */

#include "Drive.h"
#include "USART.h"
#include "LCD.h"
// defines---------



//Variables---------
                //define move function drive(direction,speed)
                //opcode 137 [velocity high byte][velocity low byte][radius high byte][radius low byte]
                //velocity -500 - 500mm/s
                //radius -2000 - 2000mm
                // straight = 32768 04 32767 = hex 8000 or 7FFF
                //define move at distance drive(direction,speed,dist)
                // while dist < x
                //drive () <- equation

/*put into main 
* drive_straight();
* drive_square();
*/
/*
 * Convert our given parameters into high and low bits to send to the irobot
 */
int hexConvertHigh(int highBit){
  int result = highBit/256;
  return result;
}

int hexConvertLow(int lowBit){
  int result = lowBit%256;
  return result;
}

void drive(int vel, int rad){    //vel = velocity in mm/sec 0->500, rad = radius of turn
  USART_OutChar(137);
  USART_OutChar(hexConvertHigh(vel));   //send the high bit velocity
  USART_OutChar(hexConvertLow(vel));    //send the low bit velocity
  USART_OutChar(hexConvertHigh(rad));   //send the high bit radius
  USART_OutChar(hexConvertLow(rad));    //send the low bit radius
}

void driveStraight(int vel, int dist){ //vel = velocity in mm/sec 0->500, dist = distance in mm
  int i = 0;                      //might need to put in an equation to account for drift of the robot
  while(i < dist){  //good place to use encoder feedback to track how far we have moved...
    drive(vel,32768);   //32768 is a special code to make the robot go straight as per the datasheet
    i++;
  }
}



