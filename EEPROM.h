/*! @file EEPROM.h
 *
 *  @brief Definition of variables for EEPROM access and modification.
 *
 *  This contains the necessary #includes for access to the internal EEPROM
 *  memory on the PIC.
 * 
 *  @note Implementation detail for the EEPROM write and read functionality 
 *  is defined in the <xc.h> library. Within the context of this project, it is
 *  best to use the macros 'EEPROM_WRITE' 'EEPROM_READ' to save a level of stack.
 *  This does however, take up slightly more memory on the PIC.
 *
 *  TODO: Need to determine whether using (or not) using the macro will actually
 *  save a stack level.
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
#define EEPROM_SONG1_ADDR 0x00
#define EEPROM_SONG2_ADDR 0x10
#define EEPROM_SONG_SIZE 16 //Each song can be 16 notes

#define EEPROM_MAP_ADDR 0x20
#define EEPROM_MAP_SIZE 10 //TODO: must determine map size

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */