/*! @file BNT.h
 *
 *  @brief Routines for controlling buttons on the PIC16F87XA.
 *
 *  @author A.Pope
 *  @date 02-08-2016
 */
#ifndef BNT_H
#define	BNT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "types.h"

/* Definition of push buttons and corresponding port pins */
#define BNT_PB1 !RB2
#define BNT_PB2 !RB3
#define BNT_PB3 !RB4
#define BNT_PB4 !RB5
#define BNT_DEB_COUNT 10

typedef void (*buttonCBFunc) (void);    /* Used within the button struct. It is a function that is invoked upon button press */

typedef struct
{
  volatile bool bntPressed;             /* Indicates if button is pressed */
  volatile bool bntReleased;            /* Indicates if button is released */
  volatile unsigned int bntDebounceCnt; /* Used to debounce the button */
  buttonCBFunc cm;                      /* Function to invoke on press */
} button_t;

/*! @brief Sets up the button module before first use.
 *
 *  @return TRUE if the module was successfully initialized.
 */
bool BNT_Init(void);

/*! @brief Debounces a specific button.
 *
 *  @param button - The button to debounce
 *  @note Assumes that DEB_Init has been called.
 */
void BNT_Debounce(button_t *button);

/*! @brief Resets the debouncing process for a button.
 *
 *  @param button - The button to reset
 *  @note Assumes that DEB_Init has been called.
 */
void BNT_ResetDebounce(button_t *button);

#ifdef	__cplusplus
}
#endif

#endif	/* BNT_H */

