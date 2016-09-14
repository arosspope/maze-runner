/*! @file BNT.c
 *
 *  @brief Routines for controlling buttons on the PIC16F87XA.
 *
 *  @author A.Pope
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
  /* First of all check if the button debounce count hasn't already reached zero.
   * 
   * If it has reached zero, it means that it has yet to be reset, and that the
   * user is still holding the button down. (we do not want to continuing asserting
   * bnt pressed in this case)
   */
  if (button->bntDebounceCnt != 0) {
    //Decrement the Debounce counter
    button->bntDebounceCnt--;
    if (button->bntDebounceCnt == 0) {
      //Button has reached required debounce time, set bnt pressed to true
      button->bntPressed = true;
      button->bntReleased = false;
    }
  }
}

void BNT_ResetDebounce(button_t *button) {
  //Reset the debounce timer and set button to released
  button->bntDebounceCnt = BNT_DEB_COUNT;
  button->bntReleased = true;
}