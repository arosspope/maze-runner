/*! @file LCD.h
 *
 *  @brief LCD routines for the PIC16F87XA.
 *
 *  This contains the functions for printing information to the LCD peripheral.
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#ifndef LCD_H
#define	LCD_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "types.h"

typedef enum {
    TOP_LEFT    = 0x00,
    TOP_RIGHT   = 0x08,
    BM_LEFT     = 0x40,
    BM_RIGHT    = 0x48
}TSCREEN_AREA; /* Defines positions on the LCD screen */    

/*! @brief Sets up the LCD before first use.
 *
 *  @return bool - TRUE if the LCD were successfully initialized.
 */
bool LCD_Init(void);

/*! @brief Prints a string at a certain position on the LCD.
 *
 *  @param data - The int to print
 *  @param area - The area to put the string in.
 *  @note Assumes that LCD_Init has been called.
 */
void LCD_PrintInt(signed int data, TSCREEN_AREA area);

/*! @brief Prints a string at a certain position on the LCD.
 *
 *  @param string - The string to Print
 *  @param area - The area to put the string in.
 *  @note Assumes that LCD_Init has been called.
 */
void LCD_PrintStr(const char * string, TSCREEN_AREA area);
#ifdef	__cplusplus
}
#endif

#endif	/* LCD_H */

