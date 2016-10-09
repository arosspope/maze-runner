/*! @file LCD.c
 *
 *  @brief LCD routines for the PIC16F87XA.
 *
 *  This contains the functions for printing information to the LCD peripheral.
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "LCD.h"

/* Definition of LCD Control pins */
#define LCD_DATA PORTD
#define EN PORTEbits.RE2
#define RW PORTEbits.RE1
#define RS PORTEbits.RE0

/*! @brief Writes a control sequence to the LCD screen.
 *
 *  @param databyte - The control sequence to send.
 */
static void writeControl(unsigned char databyte) {
  EN = 0;
  RW = 0;
  RS = 0; //Set control bits
  LCD_DATA = databyte;
  EN = 1;
  EN = 0; //Reset control bits

  __delay_ms(2);
}

/*! @brief Writes a character to the LCD screen.
 *
 *  @param c - The character to write.
 */
static void writeChar(unsigned char c) {
  EN = 0;
  RW = 0;
  RS = 1;
  LCD_DATA = c;
  EN = 1;
  EN = 0;

  __delay_ms(1);
}

/*! @brief Moves the LCD cursor to a place on the screen.
 *
 *  @param address - The address to move the cursor too.
 */
static void setCursor(unsigned char address) {
  address |= 0b10000000; //format address command using mask
  writeControl(address); //write address command
}

void LCD_PrintInt(signed int data, TSCREEN_AREA area) {
  int i;
  char str[10];

  /* Depending if text will be left or right justified, convert int to string */
  if (area == TOP_LEFT || area == BM_LEFT) {
    sprintf(str, "%-*d", 8, data);
  } else {
    sprintf(str, "%*d", 8, data);
  }

  /* Move the cursor to specified area on screen and write the string. */
  setCursor(area);
  for (i = 0; i < strlen(str); i++) {
    writeChar(str[i]);
  }
}

void LCD_PrintStr(const char * string, TSCREEN_AREA area){
  int i;
  char str[10];
  
  /* Depending if text will be left or right justified, put string in */
  if (area == TOP_LEFT || area == BM_LEFT) {
    sprintf(str, "%-*s", 8, string);
  } else {
    sprintf(str, "%*s", 8, string);
  }

  /* Move the cursor to specified area on screen and write the string. */
  setCursor(area);
  for (i = 0; i < strlen(str); i++) {
    writeChar(str[i]);
  }
}

bool LCD_Init(void) {
  //Clear PORTD/E and set to Output
  PORTD = 0;
  TRISD = 0b00000000;
  PORTE = 0;
  TRISE = 0b00000000;

  //LCD Init
  writeControl(0b00000001); //clear display
  writeControl(0b00111000); //set up display
  writeControl(0b00001100); //turn display on
  writeControl(0b00000110); //move to first digit
  writeControl(0b00000010); //entry mode setup

  return true;
}