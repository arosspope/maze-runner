/*  @file Drive.c
 *
 *  drive speed, distance, direction
 *
 *
 *  @author Kate Leone (sn: 12011843)
 *  @date 04-09-2016
 */

#include "Drive.h"
#include "IROBOT.h"
#include "IR.h"
#include "USART.h"
#include "SM.h"
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

drive(int vel, int dir){    //vel = velocity in mm/sec -500->500, dir = direction in degrees from forward
  USART_OutChar(137);
  USART_OutChar(hexConvertHigh(vel));
  USART_OutChar(hexConvertLow(vel));
  USART_OutChar(hexConvertHigh(dir));
  USART_OutChar(hexConvertLow(dir));
}

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
