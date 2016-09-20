/*!
 * @file main.c
 * @brief
 *         Main module.
 *         This module contains user's application code.
 *  @author A.Pope
 *  @date 02-08-2016
 */
#include <pic.h>
#include <xc.h>

/* User defined libaries */
#include "LED.h"
#include "LCD.h"
#include "IR.h" //TODO: Will main actually need to know about the IR sensor? (should we read IR at all?)
#include "BNT.h"
#include "IROBOT.h"
#include "types.h"

#pragma config BOREN = OFF, CPD = OFF, WRT = OFF, FOSC = HS, WDTE = OFF, CP = OFF, LVP = OFF, PWRTE = OFF
#define TMR0_VAL           100  //TMR0 will count from 0 to 100 in 1ms
#define IR_DELAY           1000 //IR Distance measurement delay of 1s
#define DEBOUNCE_DELAY     5    //Debounce delay of 5ms
#define HEARTBEAT_DELAY    500  //Heartbeat of 500ms
bool IR_FLAG = false;           //Used to signal the main loop when to perform a distance measurement

/* The list of all buttons in the system
 * Note - Rather than using the callback, we will hardcode what happens on button
 * press into the main loop. This is to save a level of stack depth in the code.
 */
button_t buttonList[] = {
  {false, false, BNT_DEB_COUNT, 0},
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
      irCnt = 0;
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

/*! @brief Initialises the whole system
 *
 *  @returns bool - TRUE If init was successfull
 */
bool systemInit(void){
  bool success;

  /* Note: LCD_Init() must be called twice, as on a power reset
   * the module does not init correctly due to weird timing issues
   * and state of registers.
   */
  success = BNT_Init() && LED_Init() && LCD_Init() && IR_Init() && IROBOT_Init()
            && timerInit() && LCD_Init();

  return success;
}

void main(void) {
  bool initOk;

  di(); //Globally disable interrupts during systemInit
  initOk = systemInit();

  if (initOk)
  {
    ei();           //Globally Enable system wide interrupts
    IROBOT_Start(); //Send startup codes to IROBOT
    
    while (1)
    {
      LCD_PrintStr("REST", TOP_LEFT); //By default, the robot is in 'REST' mode

      //IR has a refresh rate of 1HZ in normal operation mode (standby)
      if (IR_FLAG) {
        LCD_PrintInt((int) IR_Measure(), TOP_RIGHT);  //Print in mm
        IR_FLAG = false;
      }

      //Check to see if button was pressed
      if(buttonList[0].bntPressed){
        buttonList[0].bntPressed = false;
        
        //TODO: Test code - for wall follow
        LCD_PrintStr("WALL", TOP_LEFT); //Print the MODE
        IROBOT_WallFollow();
        IROBOT_Test();
      }
    }
  }

  for (;;); //Safety end loop incase program does not successfully init
}