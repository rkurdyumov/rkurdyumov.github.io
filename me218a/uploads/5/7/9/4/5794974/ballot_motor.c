//#define BALLOT_MOTOR_TEST
/**************************************************************************

Ballot Motor Module
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include <timers12.h>
#include "ballot_motor.h"
#include "ADS12.h"
#include "PWMS12.h"
/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/

void EjectBallot(void) {
  
  // Set motor running
  PWMS12_SetDuty(40, 2);  
}

void StopBallotMotor(void) {

  // Stop motor
  PWMS12_SetDuty(0, 2); 
}
 
#ifdef BALLOT_MOTOR_TEST

void main(void) {

  TMRS12_Init(TMRS12_RATE_1MS);
  PWMS12_Init();
  printf("stopping ballot motor\r\n");
  StopBallotMotor();
  
  // motor running timer
  TMRS12_InitTimer(1, 185);
  printf("Motor initialized\r\n");
  
  // Run motor until timer expires, then stop
  while(1){
	    
	    printf("Running motor\r\n");
	    EjectBallot();
	    if(TMRS12_IsTimerExpired(1))
	    {
	    StopBallotMotor();
	    break;
	    }  
	}
  
}
#endif