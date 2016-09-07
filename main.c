/*!
 * @file main.c
 * @brief
 *         Main module.
 *         This module contains user's application code.
 *  @author Andrew Pope (sn: 11655949)
 *  @date 02-08-2016
 */
#include <pic.h>
#include <xc.h>

/* User defined libaries */
#include "LED.h"
#include "LCD.h"
#include "IR.h"
#include "BNT.h"
#include "IROBOT.h"
#include "types.h"

#pragma config BOREN = OFF, CPD = OFF, WRT = OFF, FOSC = HS, WDTE = OFF, CP = OFF, LVP = OFF, PWRTE = OFF
#define TMR0_VAL           100  //TMR0 will count from 0 to 100 in 1ms
#define IR_DELAY           1000 //IR Distance measurement delay of 1s
#define DEBOUNCE_DELAY     5    //Debounce delay of 5ms
#define HEARTBEAT_DELAY    500  //Heartbeat of 500ms
bool IR_FLAG = false;           //Used to signal the main loop when to perform a distance measurement

/* Callback functions for each button within the system */
void b1CB(void) {
  IROBOT_Scan360();
}

void b2CB(void) {
  //TODO: IROBOT - 4m straight line manoeuvre
  IROBOT_DriveStraight();
}

void b3CB(void) {
  //TODO: IROBOT - 1m square box manoeuvre
  IROBOT_Test();
}

void b4CB(void) {
  //TODO: IROBOT - Parallel wall manoeuvre
  // //TODO: Must remove, initiates test mode (figure-8 pattern)
}

/* The list of all buttons in the system */
button_t buttonList[] = {
  {false, false, BNT_DEB_COUNT, &b1CB},
  {false, false, BNT_DEB_COUNT, &b2CB},
  {false, false, BNT_DEB_COUNT, &b3CB},
  {false, false, BNT_DEB_COUNT, &b4CB}
};

void interrupt isr(void) {
  static unsigned int debCnt = 0;
  static unsigned int hbCnt = 0;
  static unsigned int irCnt = 0;

  if (INTCONbits.T0IF && INTCONbits.T0IE) {
    INTCONbits.T0IF = 0; // Clear Flag for Timer0 Interrupt
    TMR0 = TMR0_VAL;     // Reset timer 0
    debCnt++; hbCnt++; irCnt++;

    //Check if system is ready to perform another measurement
    if (!(irCnt % IR_DELAY)) {
      IR_FLAG = true;
    }

    //Check to flash 'heartbeat' LED
    if (!(hbCnt % HEARTBEAT_DELAY)) {
      hbCnt = 0;
      LED_0 = !LED_0;
    }

    //Check if buttons require debouncing
    if (!(debCnt % DEBOUNCE_DELAY)) {
      debCnt = 0;

      if (BNT_PB1) { //Button 1
        BNT_Debounce(&buttonList[0]); //If button is currently pressed debounce
      } else {
        BNT_ResetDebounce(&buttonList[0]); //If released, reset the debounce count
      }

      if (BNT_PB2) { //Button 2
        BNT_Debounce(&buttonList[1]);
      } else {
        BNT_ResetDebounce(&buttonList[1]);
      }

      if (BNT_PB3) { //Button 3
        BNT_Debounce(&buttonList[2]);
      } else {
        BNT_ResetDebounce(&buttonList[2]);
      }

      if (BNT_PB4) { //Button 4
        BNT_Debounce(&buttonList[3]);
      } else {
        BNT_ResetDebounce(&buttonList[3]);
      }
    }
  }
}

/*! @brief Initialises Timer0 appropriately.
 *
 *  @returns bool - TRUE If init was successfull
 */
bool timerInit(void) {
  /* We have a 20MHz Internal clock
   * If we set timer0 prescaler to 32, it will count from 0 to 100 in 1ms
   */
  TMR0 = TMR0_VAL;
  OPTION_REGbits.T0CS = 0; //Ensure clock is running on Internal CLKO
  OPTION_REGbits.PSA = 0; //Prescaler assigned to TMR0
  OPTION_REGbits.PS2 = 1; //Set prescaler to 1:32
  OPTION_REGbits.PS1 = 0;
  OPTION_REGbits.PS0 = 0;

  INTCONbits.T0IE = 1; //Enable TMR0 Interrupt

  return true;
}

void main(void) {
  unsigned int i;

  if ( BNT_Init() && LED_Init() && LCD_Init() && IROBOT_Init() && IR_Init() && timerInit())
  {
    LCD_Init(); /* Note: LCD_Init() must be called again, as on a power reset
                 * the module does not init correctly due to weird timing issues
                 * and state of registers.
                 */
    ei();           //Globally Enable system wide interrupts
    IROBOT_Start(); //Send startup codes to IROBOT
    
    while (1)
    {
      if (IR_FLAG) {
        LCD_Print((int) IR_Measure(), TOP_RIGHT);  //Print in mm - TODO: should be in cm?
        IR_FLAG = false;
      }

      //Loop through button list, and check if button was pressed
      for (i = 0; i < sizeof (buttonList) / sizeof (button_t); i++) {
        if (buttonList[i].bntPressed) {
          //If button was pressed, invoke the appropriate callback function
          buttonList[i].bntPressed = false;
          buttonList[i].cm();
        }
      }
    }
  }

  for (;;); //Safety end loop incase program does not successfully init
}