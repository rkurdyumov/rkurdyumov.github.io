//#define CLOCK_SERVO_TEST
/**************************************************************************

Clock Servo Module

Note: the servo is activated between 450 and 2650 pulse width
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include "ServoLib.h"
#include <timers12.h>
#include "clock_servo.h"
/*--------------------------- Module Defines ----------------------------*/
#define gameTime = 30; // 30s
#define clockPeriod = 1000; // 1s

/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
static const unsigned int startClockPosition; // encoder count
static const unsigned int changePosition = 66; // completes 180 degree clock rotation
static unsigned int currentPosition;

/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/

void ResetClock(void) {
 
  // Go to start clock position
  Servo12_SetPulseWidth(5, startClockPosition);
  currentPosition = startClockPosition;
  
}

void AdvanceClock(void) {
  
  // Move clock forward
  Servo12_SetPulseWidth(5, currentPosition - changePosition);
  currentPosition -= changePosition;
}
  


#ifdef CLOCK_SERVO_TEST

/* Test harness */
void main(void) {

  unsigned int prev_time, curr_time, counter = 0;

  // Initialize port T5 to control clock
  Servo12_Init("xxSxxxxx");
  ResetClock();

  TMRS12_Init(TMRS12_RATE_1MS);

  prev_time = TMRS12_GetTime();
  
  while(1) {
    
    if(counter < 30) {
      
            curr_time = TMRS12_GetTime();
            if(curr_time - prev_time > clockPeriod) {
          
              AdvanceClock();
              prev_time = curr_time;
              counter++;  
              printf("Clock counter: %d\r\n", counter);
            }
    }
    else {
      ResetClock();
    }

  }
}

#endif