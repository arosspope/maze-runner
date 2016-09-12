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


/* define move function drive(direction,speed)
* opcode 137 [velocity high byte][velocity low byte][radius high byte][radius low byte]
* velocity -500 - 500mm/s
* radius -2000 - 2000mm
* straight = 32768 04 32767 = hex 8000 or 7FFF
* define move at distance drive(direction,speed,dist)
* while dist < x
*/
                



void drive(int16_t vel, int16_t rad){    //vel = velocity in mm/sec 0->500, rad = radius of turn
  int16union_t velBytes, radBytes, rightVel, leftVel;
  velBytes.l = vel;
  radBytes.l = rad;
  
  rightVel.l = velBytes.l;
  leftVel.l = (velBytes.l);
  
  USART_OutChar(145);
  USART_OutChar(rightVel.s.Hi); //send the high bit velocity
  USART_OutChar(rightVel.s.Lo); //send the low bit velocity
  USART_OutChar(leftVel.s.Hi); //send the high bit radius
  USART_OutChar(leftVel.s.Lo); //send the low bit radius
}

void rotate(int16_t vel, bool CW){
  int16union_t rightVel, leftVel;
  if(CW){
   leftVel.l = vel;
   rightVel.l = vel * -1;
  }else{
    leftVel.l = vel * -1;
    rightVel.l = vel;
  }
  
  USART_OutChar(145);
  USART_OutChar(rightVel.s.Hi); //send the high bit velocity
  USART_OutChar(rightVel.s.Lo); //send the low bit velocity
  USART_OutChar(leftVel.s.Hi); //send the high bit radius
  USART_OutChar(leftVel.s.Lo); //send the low bit radius
}

void driveStraight(int16_t vel, int dist){ //vel = velocity in mm/sec 0->500, dist = distance in mm
  int i;
  //@Note: At the moment, this code does not actually keep track of distance traveled.
  //       To fix this, we must get the relevant packet from the irobot that returns distance travelled
}