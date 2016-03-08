//#define BANK_SERVO_TEST
/**************************************************************************

Bank Servo Module

Note: the servo is activated between 450 and 2650 pulse width
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include "ServoLib.h"
#include <timers12.h>
#include "bank_servo.h"
/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/
 
void SetBankLevel(unsigned int left, unsigned int right) {
 
  unsigned int levelLeft =  2500 - (18*left)/30;
  unsigned int levelRight = 800 + (18*right)/30;
 
  if(left >=0 && left <= 3000 && right >=0 && right <= 3000) {

    Servo12_SetPulseWidth(4, levelLeft);
    Servo12_SetPulseWidth(3, levelRight);
  }
  else if (left > 3000 &&  right >=0 && right <= 3000) {
  	
  	Servo12_SetPulseWidth(4, 700);
    Servo12_SetPulseWidth(3, levelRight);
  }
  else if (left >=0 && left <= 3000 && right >0) {
  	
  	Servo12_SetPulseWidth(4, levelLeft);
    Servo12_SetPulseWidth(3, 2600);
  }	
  else
    printf("Error: bank level inputs <0 or >3000\r\n");
}

#ifdef BANK_SERVO_TEST

void main(void) {
  
  Servo12_Init("xxxSSxxx");
  
  printf("Press any key to set servos to 0\r\n");
  getchar();
  SetBankLevel(0,0);
    printf("Press any key to set servos to 500\r\n");
  getchar();
  SetBankLevel(500,500);
    printf("Press any key to set servos to 1000\r\n");
  getchar();
  SetBankLevel(1000,1000);
    printf("Press any key t   o set servos to 1500\r\n");
  getchar();
  SetBankLevel(1500,1500);
    printf("Press any key to set servos to 2000\r\n");
  getchar();
   SetBankLevel(2000,2000);
       printf("Press any key to set servos to 2500\r\n");
  getchar();
   SetBankLevel(2500,2500);
       printf("Press any key to set servos to 3000\r\n");
  getchar();
   SetBankLevel(3000,3000);
}
#endif