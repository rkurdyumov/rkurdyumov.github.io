//#define BALLOT_SENSOR_TEST
/**************************************************************************

Ballot Sensor Module

*/
/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <me218_c32.h>
#include "ADS12.h"
#include <timers12.h>
#include "ballot_sensor.h"

/*--------------------------- Module Defines ----------------------------*/
#define debounceInterval = 100; //ms

/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
/*-------------------------- Module Functions ---------------------------*/
/*----------------------------- Module Code -----------------------------*/

BALLOT_EVENT_t CheckBallotState(void) {
	
	// Read current ballot state
	BALLOT_EVENT_t currState; 
	
	// Test if bit high
	if(PORTE & BIT0HI)
	  currState = NO_EVENT;
	else
	  currState = BALLOT_SENSED;
	
	return currState;
}

#ifdef BALLOT_SENSOR_TEST
void main(void) {
	
	printf("Ballot Sensor initialized\r\n");
	while(1) {
		switch(CheckBallotState()) {
			
			case NO_EVENT:
				break;
			
			case BALLOT_SENSED:
				printf("Ballot inserted\r\n");
			break;
		}
	}
}
#endif