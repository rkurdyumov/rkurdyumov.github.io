//#define REGION_BUTTONS_TESTING
/**************************************************************************

Region Buttons Module

Port M2		MUX Pin 4		Left  NW,SW
Port M3		MUX Pin 7		Left  NE,SE
Port M4		MUX Pin 9		Right NW,SW
Port M5		MUX Pin 12		Right NE,SE
Port A7		MUX Pin 1

0 1A  L1  M5
1 1B  L2  M5
2 2A  L3  M4
3 2B  L4  M4
4 3A  R1  M3
5 3B  R2  M3
6 4A  R3  M2
7 4B  R4  M2

*/
/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <me218_c32.h>
#include "ADS12.h"
#include <timers12.h>
#include "region_buttons.h"
/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
typedef enum {
  ON,
  OFF 
}BUTTON_STATE_t;

typedef enum {
  LO,
  HI
}SELECT_STATE_t;

/*-------------------------- Module Variables ---------------------------*/
static unsigned int debounceInterval = 100;
static unsigned int lastTimeLO, lastTimeHI;
static BUTTON_STATE_t lastButton[8];
static SELECT_STATE_t nextSampleState;
/*-------------------------- Module Functions ---------------------------*/
static void SelectLO(void);
static void SelectHI(void);
static BUTTON_STATE_t CheckButtonState(unsigned char buttonIndex);

/*----------------------------- Module Code -----------------------------*/

static void SelectLO() {
  
  PTAD &= BIT7LO;
}

static void SelectHI() {
  
  PTAD |= BIT7HI;
}

void InitializeButtons(void){
	
	unsigned char i;

	SelectLO(); // set initial state of L1, L3, R1, R3 buttons
	for (i=0; i<4; i++) {
	   lastButton[2*i] = OFF;
	}
	lastTimeLO = TMRS12_GetTime();
	SelectHI(); // set initial state of L2, L4, R2, R4 buttons
	for (i=0; i<4; i++) {
	   lastButton[2*i + 1] = OFF;
	}
	lastTimeHI = TMRS12_GetTime();
	SelectLO(); // Go back to sampling L1, L3, R1, R3 buttons

  nextSampleState = LO;
}

static BUTTON_STATE_t CheckButtonState(unsigned char buttonIndex) {

  BUTTON_STATE_t returnState;

  // 1a and 1b
  if(buttonIndex == 0 || buttonIndex == 1)
    returnState = PTM & BIT5HI;
  // 2a and 2b
  else if(buttonIndex == 2 || buttonIndex == 3)
    returnState = PTM & BIT4HI;
  // 3a and 3b
  else if(buttonIndex == 4 || buttonIndex == 5)
    returnState = PTM & BIT3HI;
  // 4a and 4b
  else if(buttonIndex == 6 || buttonIndex == 7)
    returnState = PTM & BIT2HI;
  
  // Convert all nonzero returns to button not pressed
  if(returnState != 0)
    returnState = OFF;
  else
    returnState = ON;
  return returnState;
}
  
unsigned char CheckButtonPress(void){
  
  unsigned char i;	
  unsigned int currTimeLO;
  unsigned int currTimeHI;
  // Initialize buttons as unpressed
  BUTTON_STATE_t Button[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  unsigned char returnEvent = 0;
  SELECT_STATE_t currSampleState = nextSampleState;

	// sampling buttons L1, L3, R1, R3
	if(currSampleState == HI) {
		currTimeHI = TMRS12_GetTime();
		// if debounce interval has expired
		if(currTimeHI - lastTimeHI > debounceInterval) {
			// check current button state
			for (i=0; i<4; i++)
			Button[2*i + 1]= CheckButtonState(2*i + 1);
        
			for (i=0; i<4; i++) {
				// if a button has been pressed down
				if (Button[2*i + 1] < lastButton[2*i + 1]) {
					// return which button changed
					returnEvent = (2*i+1)+1;
					// update the last button state
					lastButton[2*i + 1] = Button[2*i + 1];
				} 
				// if a button has been released
				else if(Button[2*i + 1] > lastButton[2*i + 1]) {
					// update the last button state
					lastButton[2*i + 1] = Button[2*i + 1];
				}	
			}
			// update the last HI time
			lastTimeHI = currTimeHI;
        }
	}
	// sampling buttons L2, L4, R2, R4
	if(currSampleState == LO) {
		currTimeLO = TMRS12_GetTime();
		// if debounce interval has expired
		if(currTimeLO - lastTimeLO > debounceInterval) {
      
			for (i=0; i<4; i++)
			Button[2*i] = CheckButtonState(2*i);
        
			for (i=0; i<4; i++) {
				// if a button has been pressed down
				if (Button[2*i] < lastButton[2*i]) {
					// return which button changed
					returnEvent = (2*i)+1;
					// update the last button state
					lastButton[2*i] = Button[2*i];
				} 
				// if a button has been released
				else if(Button[2*i] > lastButton[2*i]) {
					// update the last button state
					lastButton[2*i] = Button[2*i];
				}
			}
		    lastTimeLO = currTimeLO;	
		}
	}
	
	if(currSampleState == HI) {
	  nextSampleState = LO;
	  SelectLO();
	}
	if(currSampleState == LO) {
	  nextSampleState = HI;
	  SelectHI();
	}
	return returnEvent;
}

/*---------------------------- Test Harness -----------------------------*/

#ifdef REGION_BUTTONS_TESTING

void main(void) {
  unsigned char buttonEvent;
  TMRS12_Init(TMRS12_RATE_1MS);
 
  //Set ports M2,M3,M4,M5 to input and A7 to output
	ADS12_Init("OOOOOOOO");
	DDRM &= BIT2LO | BIT3LO | BIT4LO | BIT5LO;
  printf("Initialized ports\r\n");
  
  InitializeButtons();
	  
	while(1) {
		buttonEvent = CheckButtonPress();
		if(buttonEvent != 0)
		  printf("Button: %d\r\n", buttonEvent);
	}
}
#endif /*TESTING*/

/*----------------------------- End of File -----------------------------*/