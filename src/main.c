/*!
 * @file main.c
 * @brief Main module.
 *         
 * Main entry point to the program.
 * @author A.Pope
 * @date 02-08-2016
 */
#include <pic.h>
#include <xc.h>

/* User defined libaries */
#include "LED.h"
#include "LCD.h"
#include "BNT.h"
#include "IROBOT.h"
#include "types.h"

#pragma config BOREN = OFF, CPD = OFF, WRT = OFF, FOSC = HS, WDTE = OFF, CP = OFF, LVP = OFF, PWRTE = OFF
#define TMR0_VAL           100  //TMR0 will count from 0 to 100 in 1ms
#define DEBOUNCE_DELAY     5    //Debounce delay of 5ms
#define HEARTBEAT_DELAY    500  //Heartbeat of 500ms

/* The list of all buttons in the system
 * Note - Rather than using the callback, we will hardcode what happens on button
 * press into the main loop. This is to save a level of stack depth in the code.
 */
button_t buttonList[] = {
  {false, false, BNT_DEB_COUNT, 0},
};

/*! @brief Interrupt Service Routine for the PIC
 *  @note All interrupts in the PIC must be serviced in this routine 
 */
void interrupt isr(void) {
  static unsigned int debCnt = 0;
  static unsigned int hbCnt = 0;
  static unsigned int irCnt = 0;

  if (INTCONbits.T0IF && INTCONbits.T0IE) {
    INTCONbits.T0IF = 0; // Clear Flag for Timer0 Interrupt
    TMR0 = TMR0_VAL;     // Reset timer 0
    debCnt++; hbCnt++;

    //Check to flash 'heartbeat' LED
    if (!(hbCnt % HEARTBEAT_DELAY)) {
      hbCnt = 0;
      LED_0 = !LED_0;
    }

    //Check if button require debouncing
    if (!(debCnt % DEBOUNCE_DELAY)) {
      debCnt = 0;

      if (BNT_PB1) { //Button 1
        BNT_Debounce(&buttonList[0]); //If button is currently pressed debounce
      } else {
        BNT_ResetDebounce(&buttonList[0]); //If released, reset the debounce count
      }
    }
  }
}

/*! @brief Initializes Timer0 appropriately.
 *
 *  @returns TRUE - If init was successful
 */
bool timerInit(void) {
  /* We have a 20MHz Internal clock
   * If we set timer0 pre-scaler to 32, it will count from 0 to 100 in 1ms
   */
  TMR0 = TMR0_VAL;
  OPTION_REGbits.T0CS = 0;  //Ensure clock is running on Internal CLKO
  OPTION_REGbits.PSA = 0;   //Pre-scaler assigned to TMR0
  OPTION_REGbits.PS2 = 1;   //Set pre-scaler to 1:32
  OPTION_REGbits.PS1 = 0;
  OPTION_REGbits.PS0 = 0;

  INTCONbits.T0IE = 1; //Enable TMR0 Interrupt

  return true;
}

/*! @brief Initializes the whole system
 *
 *  @returns TRUE If init was successful
 */
bool systemInit(void){
  bool success;

  /* Note: LCD_Init() must be called twice, as on a power reset
   * the module does not init correctly due to weird timing issues
   * and state of registers.
   */
  success = BNT_Init() && LED_Init() && LCD_Init() && IROBOT_Init()
            && timerInit() && LCD_Init();

  return success;
}

void main(void) {
  bool initOk;

  di(); //Globally disable interrupts during systemInit
  initOk = systemInit();

  if (initOk)
  {
    ei();           //Globally Enable system wide interrupts if init okay
    IROBOT_Start(); //Send startup codes to IROBOT
    
    while (1)
    {
      //Check to see if button was pressed
      if(buttonList[0].bntPressed)
      {
        buttonList[0].bntPressed = false;
        IROBOT_MazeRun(); //The robot will initiate the maze-run routine
      }
    }
  }

  for (;;); //Safety end loop incase program does not successfully init
}