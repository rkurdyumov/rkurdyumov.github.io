//#define VIBRATE_MOTOR_TEST
/**************************************************************************

Vibrate Motor Module
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include <timers12.h>
#include "ADS12.h"
#include "vibrate_motor.h"
#include "PWMS12.h"
/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/

void VibrateMotor(VIBRATE_MOTOR_t motor) {
  
  printf("motor vibrated\r\n");
  if (motor == LEFT_MOTOR)
    PWMS12_SetDuty(100, 1);  // motor running
  else if (motor == RIGHT_MOTOR)
    PWMS12_SetDuty(100, 0);
  else if (motor == BOTH_MOTORS) {
    PWMS12_SetDuty(100, 1);  
    PWMS12_SetDuty(100, 0);
  }
}

void StopVibrateMotor(VIBRATE_MOTOR_t motor) {

  printf("vibrate motor stopped\r\n");
  if (motor == LEFT_MOTOR)
    PWMS12_SetDuty(0, 1);  // motor stopped
  else if (motor == RIGHT_MOTOR)
    PWMS12_SetDuty(0, 0);
  else if (motor == BOTH_MOTORS) {
    PWMS12_SetDuty(0, 1);  
    PWMS12_SetDuty(0, 0);
  }
}

 
#ifdef VIBRATE_MOTOR_TEST

void main(void) {

  TMRS12_Init(TMRS12_RATE_1MS);
  PWMS12_Init();
  StopVibrateMotor(BOTH_MOTORS);
  
  TMRS12_InitTimer(1, 500);
  printf("Motors initialized\r\n");
  
  while(1){
	    
	    VibrateMotor(BOTH_MOTORS);
	    if(TMRS12_IsTimerExpired(1))
	    {
	    StopVibrateMotor(BOTH_MOTORS);
	    break;
	    }  
	}
  
}
#endif