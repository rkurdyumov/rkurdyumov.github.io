//#define CARD_SENSOR_TEST
/**************************************************************************

Card Sensor Module
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include <timers12.h>
#include "ADS12.h"
#include "card_sensor.h"

/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
static CARD_STATE_t lastLeftCardState, lastRightCardState;
static const unsigned int insertTimeInterval = 1000;
static unsigned int lastLeftTime, lastRightTime;
static unsigned int leftNoCard;
static unsigned int leftRed;
static unsigned int leftGreen;
static unsigned int leftBlue;
static unsigned int rightNoCard;
static unsigned int rightRed;
static unsigned int rightGreen;
static unsigned int rightBlue;

/*-------------------------- Module Functions ---------------------------*/
static CARD_STATE_t AnalogToLeftCardState(unsigned int analog);
static CARD_STATE_t AnalogToRightCardState(unsigned int analog);
/*----------------------------- Module Code -----------------------------*/

void InitializeCardSensor(void) {

  // Initialize
  lastLeftCardState = NO_CARD;
  lastRightCardState = NO_CARD;
  lastLeftTime = TMRS12_GetTime();
  lastRightTime = TMRS12_GetTime();
}

// Record the analog counts for calibrating each card sensor
void SetCardCalibration(unsigned int sideCard) {
	
	switch(sideCard) {

		case 1:
			leftNoCard = ADS12_ReadADPin(2);
			break;
		
		case 2:
			leftGreen = ADS12_ReadADPin(2);
			break;
			
		case 3:
			leftBlue= ADS12_ReadADPin(2);
			break;
			
		case 4:

			leftRed = ADS12_ReadADPin(2);
			break;	
		
		case 5:
			rightNoCard = ADS12_ReadADPin(3);
			break;
		
		case 6:
			rightGreen = ADS12_ReadADPin(3);
			break;
		
		case 7:
			rightBlue = ADS12_ReadADPin(3);
			break;
		
		case 8:
			rightRed = ADS12_ReadADPin(3);
			break;
		}
	
	
}

// Convert analog counts to a left card sensor state
CARD_STATE_t AnalogToLeftCardState(unsigned int analog) {
  
  CARD_STATE_t returnState;
  
  if (analog < (leftNoCard + leftBlue)/2)
    	  returnState = NO_CARD;
 	else if (analog > (leftRed + leftGreen)/2)
    	  returnState = RED_CARD;
 	else if (analog > (leftBlue + leftGreen)/2 && analog < (leftRed + leftGreen)/2)
    	  returnState = GREEN_CARD;
 	else if (analog > (leftBlue + leftNoCard)/2  && (leftBlue + leftGreen)/2)
    	  returnState = BLUE_CARD;
  
  return returnState;
}

// Convert analog counts to a right card sensor state
CARD_STATE_t AnalogToRightCardState(unsigned int analog) {
  
  CARD_STATE_t returnState;
  
  if (analog < (rightNoCard + rightBlue)/2)
    	  returnState = NO_CARD;
 	else if (analog > (rightRed + rightGreen)/2)
    	  returnState = RED_CARD;
 	else if (analog > (rightBlue + rightGreen)/2 && analog < (rightRed + rightGreen)/2)
    	  returnState = GREEN_CARD;
 	else if (analog > (rightBlue + rightNoCard)/2  && (rightBlue + rightGreen)/2)
    	  returnState = BLUE_CARD;
  
  return returnState;
}

// Check the current left card state
CARD_STATE_t CheckLeftCardState(void) {
	
	CARD_STATE_t returnState = lastLeftCardState;
	unsigned int currLeftCardState = AnalogToLeftCardState(ADS12_ReadADPin(2));
	unsigned int currTime = TMRS12_GetTime();
  
  // If card state has changed
  if (currLeftCardState != lastLeftCardState) {
  // If insert interval has expired
    if (currTime - lastLeftTime > insertTimeInterval) {
		// Update current state
		returnState = currLeftCardState;
    	lastLeftCardState = currLeftCardState;
    	lastLeftTime = currTime;
    } 
  } 
  else
    lastLeftTime = currTime;
  
	return returnState;
}

// Check the current right card state
CARD_STATE_t CheckRightCardState(void) {
	
	CARD_STATE_t returnState = lastRightCardState;
	unsigned int currRightCardState = AnalogToRightCardState(ADS12_ReadADPin(3));
	unsigned int currTime = TMRS12_GetTime();
  
  if (currRightCardState != lastRightCardState) {
    if (currTime - lastRightTime > insertTimeInterval) {
      returnState = currRightCardState;
    	lastRightCardState = currRightCardState;
    	lastRightTime = currTime;
    } 
  } 
  else
    lastRightTime = currTime;
  
	return returnState;
}

#ifdef CARD_SENSOR_TEST
void main(void) {
	
	CARD_STATE_t currLeftState, currRightState;
	TMRS12_Init(TMRS12_RATE_1MS);
	ADS12_Init("OOOOAAOO");
	InitializeCardSensor();
	while(1) {	
	
	  printf("%d %d\r\n", ADS12_ReadADPin(2), ADS12_ReadADPin(3));
    //printf("%d\r\n", AnalogToRightCardState(ADS12_ReadADPin(2)));
    
    /*
    currLeftState= CheckLeftCardState();
    currRightState = CheckRightCardState();
    printf("%d %d\r\n", currLeftState, currRightState);
	*/
	}
	
}
 
#endif