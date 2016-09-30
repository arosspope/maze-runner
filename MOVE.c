/*! @file IROBOT.c
 *
 *  @brief Module to move the iRobot.
 *
 *  This contains the functions for moving the iRobot through the create open
 *  interface.
 *
 *  @author A.Pope, K.Leone, C.Stewart, J.Lynch, A.Truong
 *  @date 02-09-2016
 */
#include "USART.h"
#include "OPCODES.h"
#include "MOVE.h"

bool MOVE_Init(void){
  return true; //No initialisation needed for the move module
}

int16_t MOVE_GetDistMoved(void){
  uint16union_t rxdata;
  
  USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_DIST);
  rxdata.s.Hi = USART_InChar();
  rxdata.s.Lo = USART_InChar();
  
  return (int16_t) rxdata.l;
}

bool MOVE_Straight(int16_t velocity, uint16_t distance){
  uint16union_t rxdata;
  TSENSORS sensStatus;
  int16_t distanceTravelled = 0;
  bool sensorTrig = false;

  MOVE_GetDistMoved(); //Reset distance encoders on the iRobot
  MOVE_DirectDrive(velocity, velocity); //Tell the IROBOT to drive straight at speed

  //Let the robot drive until it reaches the desired distance
  while((distanceTravelled < distance) && !sensorTrig){
    //Get Distance Moved
    if(velocity >= 0){
      distanceTravelled += MOVE_GetDistMoved(); //Positive velocity returns positive distance
    } else {
      distanceTravelled += (MOVE_GetDistMoved() * -1); //Negative vel returns neg dist (must normalise)
    }

    sensorTrig = MOVE_CheckSensor(&sensStatus);
  }

  MOVE_DirectDrive(0, 0); //Tell the IROBOT to stop moving

  return sensorTrig;
}

bool MOVE_Rotate(uint16_t velocity, uint16_t angle, TDIRECTION dir){
  uint16union_t rxdata;
  TSENSORS sensStatus;
  int16_t angleMoved = 0;
  bool sensorTrig = false;

  //Get current angle moved to reset the angle moved count
  USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_ANGLE);
  USART_InChar(); USART_InChar();  //Dummy read to clear recieve buffer

  if (dir == DIR_CCW){
    MOVE_DirectDrive((velocity * -1), velocity); //Make the robot turn CCW @ 210mm/s
  } else {
    MOVE_DirectDrive(velocity, (velocity * -1)); //Make the robot turn CW @ 210mm/s
  }

  while ((angleMoved < angle) && !sensorTrig)
  {
    //Get Angle since last movement
    USART_OutChar(OP_SENSORS); USART_OutChar(OP_SENS_ANGLE);
    rxdata.s.Hi = USART_InChar();
    rxdata.s.Lo = USART_InChar();

    if(dir == DIR_CCW){
      angleMoved += (int16_t) rxdata.l; //CCW direction returns positive angles
    }else{
      angleMoved += ((int16_t) rxdata.l * -1); //CW direction returns negative angles
    }

    sensorTrig = MOVE_CheckSensor(&sensStatus);
  }

  MOVE_DirectDrive(0, 0); //Tell the IROBOT to stop rotating
  return sensorTrig;
}

void MOVE_DirectDrive(int16_t leftWheelVel, int16_t rightWheelVel){
  int16union_t rightBytes, leftBytes;

  rightBytes.l = rightWheelVel;
  leftBytes.l = leftWheelVel;

  USART_OutChar(OP_DRIVE_DIRECT);
  USART_OutChar(rightBytes.s.Hi); //Send the velocity for the right wheel
  USART_OutChar(rightBytes.s.Lo);
  USART_OutChar(leftBytes.s.Hi); //Send the velocity for the left wheel
  USART_OutChar(leftBytes.s.Lo);
}

bool MOVE_CheckSensor(TSENSORS * sensors){
  uint8_t data;
  sensors->bump = false; sensors->wall = false; sensors->victim = false;

  //Tell the Robot to send back information regarding a group of sensors
  USART_OutChar(OP_QUERY);
  USART_OutChar(3);         //Get information about 3 sensors TODO: Implement victim
  USART_OutChar(OP_SENS_BUMP);
  USART_OutChar(OP_SENS_WALL);
  USART_OutChar(OP_SENS_IR);
  
  //1. Packet ID: 7 (Bump and Wheel drop)
  sensors->bump = (USART_InChar() & 0b00000011);   //We only care about the bump data so AND with mask
  
  //2. Packet ID: 8 (Wall)
  //  sensStatus->sensBits.wall = USART_InChar();

  //3. Packet ID: 17 (Infrared Byte)
//  data = USART_InChar();
//  if(data >= 248 && data <= 254)  //A particular range of IR values indicates that we are in prescense of home base
//    sensStatus->sensBits.victim = 1;
  USART_InChar(); USART_InChar();
  //return (sensStatus->sensors > 0); //A value greater than 0 indicates one of the sensors have been tripped
  return (sensors->bump || sensors->wall || sensors->victim);
}