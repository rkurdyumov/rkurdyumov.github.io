//#define FAN_TEST
/**************************************************************************

Fan Module
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include <timers12.h>
#include "ADS12.h"
#include "fan.h"
/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/

void StartFan(FAN_t fan) {
  
  switch(fan) {
    
    case LEFT_FAN:
      PTAD |= BIT4HI; 
      break;
      
    case RIGHT_FAN:
      PTAD |= BIT5HI; 
      break;
      
    case BOTH_FANS:
      PTAD |= BIT4HI;
      PTAD |= BIT5HI; 
      break;
    
  }
}

void StopFan(FAN_t fan) {

  switch(fan) {
    
    case LEFT_FAN:
      PTAD &= BIT4LO;
      break; 
      
    case RIGHT_FAN:
      PTAD &= BIT5LO; 
      break;
      
    case BOTH_FANS:
      PTAD &= BIT4LO; 
      PTAD &= BIT5LO; 
      break;
  }
}

 
#ifdef FAN_TEST
 
void main(void) {

  TMRS12_Init(TMRS12_RATE_1MS);
  ADS12_Init("OOOOOOOO"); // make Ports A4/A5 a digital output
  StartFan(BOTH_FANS);
  
  TMRS12_InitTimer(1, 1500);
  //TMRS12_InitTimer(2, 1000);
  //TMRS12_InitTimer(3, 1500);
  printf("Fans initialized\r\n");
  
  while(1){
	    
	    if(TMRS12_IsTimerExpired(1) == TMRS12_EXPIRED) {
	    StopFan(BOTH_FANS);
	    TMRS12_ClearTimerExpired(1);
	    }
	    /*
	    if(TMRS12_IsTimerExpired(1) == TMRS12_EXPIRED) {
	    StopFan(BOTH_FANS);
	    TMRS12_ClearTimerExpired(1);
	    }
	    if(TMRS12_IsTimerExpired(2) == TMRS12_EXPIRED) {
	    StartFan(BOTH_FANS);
	    TMRS12_ClearTimerExpired(2);
	    }
	    if(TMRS12_IsTimerExpired(3) == TMRS12_EXPIRED) {
	    StopFan(BOTH_FANS);
	    TMRS12_ClearTimerExpired(3);
	    }
	    */
  
  }
}
#endif