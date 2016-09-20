/*! @file EEPROM.h
 *
 *  @brief Definition of variables for EEPROM access and modification.
 *
 *  This contains the necessary #includes for access to the internal EEPROM
 *  memory on the PIC.
 * 
 *  @note Implementation detail for the EEPROM write and read functionality 
 *  is defined in the <xc.h> library.
 *
 *  TODO: Need to determine whether using (or not) using the function 'eeprom_write/read' will effect stack level
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#ifndef EEPROM_H
#define	EEPROM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

/* Starting addresses for bytes of data in FLASH */
/* Song Address Ranges */
#define EEPM_SONG_SIZE  16   //Each song can be 16 notes (16 bytes in Flash mem)
#define EEPM_SONG0_ADDR 0x00 //Address offset for song0
#define EEPM_SONG1_ADDR 0x10 //Address offset for song1
#define EEPM_SONG2_ADDR 0x20 //Address offset for song2
#define EEPM_SONG3_ADDR 0x30 //Address offset for song3

/* Maze map inforamtion*/
#define EEPM_MAP_SIZE   20   //TODO: must determine map size
#define EEPM_MAP_ADDR   0x40 //Address offset for the map

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */