/*! @file BNT.c
 *
 *  @brief Routines for controlling buttons on the PIC16F87XA.
 *
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */
#include "BNT.h"

bool BNT_Init(void) {
  //Set the relevant PORTB pins to input mode
  TRISBbits.TRISB2 = 1;
  TRISBbits.TRISB3 = 1;
  TRISBbits.TRISB4 = 1;
  TRISBbits.TRISB5 = 1;

  return true;
}

void BNT_Debounce(button_t *button) {
  //Decrement the Debounce counter
  button->bntDebounceCnt--;
  if (button->bntDebounceCnt == 0) {
    //Button has reached required debounce time, set bnt pressed to true
    button->bntPressed = true;
    button->bntReleased = false;
  }
}

void BNT_ResetDebounce(button_t *button) {
  //Reset the debounce timer and set button to released
  button->bntDebounceCnt = BNT_DEB_COUNT;
  button->bntReleased = true;
}