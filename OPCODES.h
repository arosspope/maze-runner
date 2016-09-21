/*! @file OPCODES.h
 *
 *  @brief Definition of the opcodes for the create open interface.
 *
 *  This contains the necessary #defines for opcodes used to control the create
 *  open interface.
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#ifndef OPCODES_H
#define	OPCODES_H

#ifdef	__cplusplus
extern "C" {
#endif

/* Changing mode of operation for the iRobot */
#define OP_START        128
#define OP_FULL         132
/* Codes for using song functionality */
#define OP_LOAD_SONG    140
#define OP_PLAY_SONG    141
/* Codes for Drive Functionality */
#define OP_DRIVE        137
#define OP_DRIVE_DIRECT 145
/* Codes for Sensor information */
#define OP_SENSORS      142
#define OP_QUERY        149
#define OP_SENS_VWALL   13
#define OP_SENS_BUMP    7
#define OP_SENS_GROUP   1   //Will return information about bump, wall, cliff, and virtual wall sensors
#define OP_SENS_DIST    19  //Distance travelled since last call
#define OP_SENS_ANGLE   20  //Angle turned since last call
#define OP_SONG_PLAYING 37  //Is a song currently playing

#ifdef	__cplusplus
}
#endif

#endif	/* OPCODES_H */

