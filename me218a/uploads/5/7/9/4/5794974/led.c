//#define LED_TEST
/**************************************************************************

LED Module

Port T7 : Data In
Port T6 : Storage Clock
Port M0 : Shift Clock

Array indices : LED
0   Red1
1   Green1  
2   Blue1
3   Red2
4   Green2
5   Blue2
6   Red3
7   Green3
8   Blue3
9   Red4
10  Green4
11  Blue4

Uses Timer #0

**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <me218_c32.h>
#include <timers12.h>
#include "LED.h"

/*--------------------------- Module Defines ----------------------------*/
#define SHIFT_CLOCK_TIME    4  // ms
#define STORAGE_CLOCK_TIME  29 // ms
#define NUM_LEDS            12

/*---------------------------- Module Types -----------------------------*/
typedef enum{
  WAITING_HI,   // waiting for HI shift clock pulse to end
  WAITING_LO,   // waiting for LO shift clock pulse to end
  WAITING_STORE // waiting for next storage clock pulse
}LED_State_t;

typedef enum{
  OFF,
  ON
}LED_Mode_t;

/*-------------------------- Module Variables ---------------------------*/ 
static LED_Mode_t LEDData[4][NUM_LEDS];
static unsigned char currLEDData;
static unsigned char prevLEDData;
static LED_State_t currState;
static unsigned char counter;         

/*-------------------------- Module Functions ---------------------------*/
static void DataHI(void);
static void DataLO(void);
static void ShiftClockHI(void);
static void ShiftClockLO(void);
static void StorageClockHI(void);
static void StorageClockLO(void);
/*----------------------------- Module Code -----------------------------*/

// Helper Functions

static void DataHI() {
  
  PTT |= BIT7HI; 
}

static void DataLO() {
  
  PTT &= BIT7LO; 
}

static void ShiftClockHI() {
  
  PTM |= BIT0HI;
}

static void ShiftClockLO() {
  
  PTM &= BIT0LO;
}

static void StorageClockHI() {
  
  PTT |= BIT6HI;
}

static void StorageClockLO() {
  
  PTT &= BIT6LO;
}

void InitializeLED(void){
  
  unsigned char i;
  
  ShiftClockHI();
  StorageClockHI();
  
  // Initialize all LEDs off
  TurnOffLEDs();
  
  // Make sure the LEDs flash the first data state
  currLEDData = 1;
  prevLEDData = 1;
  
  // Set the currentState
  currState = WAITING_HI;
  
  // Initialize counter
  counter = 0;
     
  //Start Waiting_HI timer
  TMRS12_InitTimer(0, SHIFT_CLOCK_TIME);
}

void CheckLEDEvents(void) {

  LED_State_t nextState;
  nextState = currState;
  
  switch(currState) {
    
    case WAITING_HI:
      if (TMRS12_IsTimerExpired(0) == TMRS12_EXPIRED) {
        if (LEDData[currLEDData - 1][NUM_LEDS - 1 - counter] == ON) {
          DataHI();
        }
        if (LEDData[currLEDData - 1][NUM_LEDS - 1 - counter] == OFF) {
          DataLO();
        }
        // Clear Expired Timer for next pass
        TMRS12_ClearTimerExpired(0);
        // Pulse Shift Clock
        ShiftClockLO();
        // Update nextState
        nextState = WAITING_LO;
        // Start Waiting LO timer
        TMRS12_InitTimer(0, SHIFT_CLOCK_TIME);
      }
      else {
        // wait for timer expiration
      }
      break;
    
    case WAITING_LO:
      if (TMRS12_IsTimerExpired(0) == TMRS12_EXPIRED) {
        // Clear Expired Timer
        TMRS12_ClearTimerExpired(0);
        // Increment counter
        counter++;
        // If we haven't finished inputting LED states
        if (counter < NUM_LEDS) {
          // Reset Shift Clock line
          ShiftClockHI();
          // Go back to Waiting for HI expiration
          nextState = WAITING_HI;
          //Start 1ms Waiting HI timer
  	      TMRS12_InitTimer(0, SHIFT_CLOCK_TIME);
        }
        // If we have finished inputting LED states, pulse storage clock line
        if (counter >= NUM_LEDS) {
          // Pulse Storage Clock line
          StorageClockLO();
          // Update nextState
          nextState = WAITING_STORE;
          //Start Waiting_STORE timer
  	      TMRS12_InitTimer(0, STORAGE_CLOCK_TIME);
        }
          
      }
      else {
        // wait for timer expiration
      }
      break;

    case WAITING_STORE:
      if (TMRS12_IsTimerExpired(0) == TMRS12_EXPIRED) {
        // Clear Expired Timer
        TMRS12_ClearTimerExpired(0);
        // Update which LED data we are loading in
        if (prevLEDData == 1)
          currLEDData = 2;
        if (prevLEDData == 2)
          currLEDData = 3;
        if (prevLEDData == 3)
          currLEDData = 4;
        if (prevLEDData == 4)
          currLEDData = 1;
        prevLEDData = currLEDData;
        // Reset counter
        counter = 0;
        // Reset shift and storage clock lines
        ShiftClockHI();
        StorageClockHI();
        // Set next state to WAITING_HI
        nextState = WAITING_HI;
        // Start Waiting HI timer
  	    TMRS12_InitTimer(0, SHIFT_CLOCK_TIME);  
      }
      else {
        // wait for timer expiration
      }
      break;
    
    
    
  }
  // Update the current state
  currState = nextState;
  
}

void UpdatePattern(LED_Color_t Standings[4][4]){
  
  unsigned char i;
  
  for (i=0; i<4; i++) {
  
    // First Place
    switch(Standings[i][0]) {
      
      case RED:
        LEDData[0][3*i] = ON;
        LEDData[1][3*i] = ON;
        LEDData[2][3*i] = ON;
        LEDData[3][3*i] = ON;
        break;
        
      case GREEN:
        LEDData[0][3*i + 1] = ON;
        LEDData[1][3*i + 1] = ON;
        LEDData[2][3*i + 1] = ON;
        LEDData[3][3*i + 1] = ON;
        break; 
        
      case BLUE:
        LEDData[0][3*i + 2] = ON;
        LEDData[1][3*i + 2] = ON;
        LEDData[2][3*i + 2] = ON;
        LEDData[3][3*i + 2] = ON;
        break;
        
      case NONE:
        break;
    }
    
    // Second Place
    switch(Standings[i][1]) {
      
      case RED:
        LEDData[0][3*i] = ON;
        LEDData[1][3*i] = OFF; 
        LEDData[2][3*i] = ON;
        LEDData[3][3*i] = OFF;
        break;
        
      case GREEN:
        LEDData[0][3*i + 1] = ON;
        LEDData[1][3*i + 1] = OFF;
        LEDData[2][3*i + 1] = ON;
        LEDData[3][3*i + 1] = OFF;        
        break; 
        
      case BLUE:
        LEDData[0][3*i + 2] = ON;
        LEDData[1][3*i + 2] = OFF;
        LEDData[2][3*i + 2] = ON;
        LEDData[3][3*i + 2] = OFF;        
        break;
        
      case NONE:
        break;
        
    }
    
    // Third Place
    switch(Standings[i][2]) {
      
      case RED:
        LEDData[0][3*i] = ON;
        LEDData[1][3*i] = ON;
        LEDData[2][3*i] = OFF;
        LEDData[3][3*i] = OFF;         
        break;
        
      case GREEN:
        LEDData[0][3*i + 1] = ON;
        LEDData[1][3*i + 1] = ON;
        LEDData[2][3*i + 1] = OFF;
        LEDData[3][3*i + 1] = OFF;        
        break; 
        
      case BLUE:
        LEDData[0][3*i + 2] = ON;
        LEDData[1][3*i + 2] = ON;
        LEDData[2][3*i + 2] = OFF;
        LEDData[3][3*i + 2] = OFF;        
        break;
        
      case NONE:
        break;
    }
    
    // Last Place
    switch(Standings[i][3]) {
      
      case RED:
        LEDData[0][3*i] = OFF;
        LEDData[1][3*i] = OFF; 
        LEDData[2][3*i] = OFF;
        LEDData[3][3*i] = OFF;        
        break;
        
      case GREEN:
        LEDData[0][3*i + 1] = OFF;
        LEDData[1][3*i + 1] = OFF;
        LEDData[2][3*i + 1] = OFF;
        LEDData[3][3*i + 1] = OFF;       
        break; 
        
      case BLUE:
        LEDData[0][3*i + 2] = OFF;
        LEDData[1][3*i + 2] = OFF;
        LEDData[2][3*i + 2] = OFF;
        LEDData[3][3*i + 2] = OFF;        
        break;
      
      case NONE:
        break;
    }
  }
}

//Flash all the LEDs of the winner, dim loser LEDs (takes Winning color as argument)
void UpdateWinnerPattern(LED_Color_t Winner){
  
  unsigned char i;
  
  switch(Winner) {
      
      case RED:
        for (i=0; i<4; i++) {
          
          LEDData[0][0] = ON;
          LEDData[1][0] = OFF; 
          LEDData[2][0] = OFF;
          LEDData[3][0] = OFF; 
          
          LEDData[0][3] = OFF;
          LEDData[1][3] = OFF; 
          LEDData[2][3] = ON;
          LEDData[3][3] = OFF; 
          
          LEDData[0][6] = OFF;
          LEDData[1][6] = OFF; 
          LEDData[2][6] = OFF;
          LEDData[3][6] = ON; 
          
          LEDData[0][9] = OFF;
          LEDData[1][9] = ON; 
          LEDData[2][9] = OFF;
          LEDData[3][9] = OFF;           
          
          LEDData[0][3*i + 1] = OFF;
          LEDData[1][3*i + 1] = OFF; 
          LEDData[2][3*i + 1] = OFF;
          LEDData[3][3*i + 1] = OFF; 
          
          LEDData[0][3*i + 2] = OFF;
          LEDData[1][3*i + 2] = OFF;
          LEDData[2][3*i + 2] = OFF;
          LEDData[3][3*i + 2] = OFF;          
        }
        break;
      
      case GREEN:
        for (i=0; i<4; i++) {
          LEDData[0][3*i] = OFF;
          LEDData[1][3*i] = OFF;
          LEDData[2][3*i] = OFF;
          LEDData[3][3*i] = OFF;           
          
          LEDData[0][1] = ON;
          LEDData[1][1] = OFF; 
          LEDData[2][1] = OFF;
          LEDData[3][1] = OFF; 
          
          LEDData[0][4] = OFF;
          LEDData[1][4] = OFF; 
          LEDData[2][4] = ON;
          LEDData[3][4] = OFF; 
          
          LEDData[0][7] = OFF;
          LEDData[1][7] = OFF; 
          LEDData[2][7] = OFF;
          LEDData[3][7] = ON; 
          
          LEDData[0][10] = OFF;
          LEDData[1][10] = ON; 
          LEDData[2][10] = OFF;
          LEDData[3][10] = OFF;  
          
          LEDData[0][3*i + 2] = OFF;
          LEDData[1][3*i + 2] = OFF;
          LEDData[2][3*i + 2] = OFF;
          LEDData[3][3*i + 2] = OFF;          
        }
        break;
      
      case BLUE:
        for (i=0; i<4; i++) {
          LEDData[0][3*i] = OFF;
          LEDData[1][3*i] = OFF;
          LEDData[2][3*i] = OFF;
          LEDData[3][3*i] = OFF;           
          
          LEDData[0][3*i + 1] = OFF;
          LEDData[1][3*i + 1] = OFF; 
          LEDData[2][3*i + 1] = OFF;
          LEDData[3][3*i + 1] = OFF;
          
          LEDData[0][2] = ON;
          LEDData[1][2] = OFF; 
          LEDData[2][2] = OFF;
          LEDData[3][2] = OFF; 
          
          LEDData[0][5] = OFF;
          LEDData[1][5] = OFF; 
          LEDData[2][5] = ON;
          LEDData[3][5] = OFF; 
          
          LEDData[0][8] = OFF;
          LEDData[1][8] = OFF; 
          LEDData[2][8] = OFF;
          LEDData[3][8] = ON; 
          
          LEDData[0][11] = OFF;
          LEDData[1][11] = ON; 
          LEDData[2][11] = OFF;
          LEDData[3][11] = OFF;         
        }
        break;
    
  }
}

// Light up the LEDs of the selected player
void DisplayOnePlayerColor(LED_Color_t player){
  
  unsigned char i;
  
  switch(player) {
      
      case RED:
        for (i=0; i<4; i++) {
          
          LEDData[0][3*i] = ON;
          LEDData[1][3*i] = ON; 
          LEDData[2][3*i] = ON;
          LEDData[3][3*i] = ON;         
          
          LEDData[0][3*i + 1] = OFF;
          LEDData[1][3*i + 1] = OFF; 
          LEDData[2][3*i + 1] = OFF;
          LEDData[3][3*i + 1] = OFF; 
          
          LEDData[0][3*i + 2] = OFF;
          LEDData[1][3*i + 2] = OFF;
          LEDData[2][3*i + 2] = OFF;
          LEDData[3][3*i + 2] = OFF;          
        }
        break;
      
      case GREEN:
        for (i=0; i<4; i++) {
          LEDData[0][3*i] = OFF;
          LEDData[1][3*i] = OFF;
          LEDData[2][3*i] = OFF;
          LEDData[3][3*i] = OFF;           
          
          LEDData[0][3*i + 1] = ON;
          LEDData[1][3*i + 1] = ON; 
          LEDData[2][3*i + 1] = ON;
          LEDData[3][3*i + 1] = ON;
          
          LEDData[0][3*i + 2] = OFF;
          LEDData[1][3*i + 2] = OFF;
          LEDData[2][3*i + 2] = OFF;
          LEDData[3][3*i + 2] = OFF;          
        }
        break;
      
      case BLUE:
        for (i=0; i<4; i++) {
          LEDData[0][3*i] = OFF;
          LEDData[1][3*i] = OFF;
          LEDData[2][3*i] = OFF;
          LEDData[3][3*i] = OFF;           
          
          LEDData[0][3*i + 1] = OFF;
          LEDData[1][3*i + 1] = OFF; 
          LEDData[2][3*i + 1] = OFF;
          LEDData[3][3*i + 1] = OFF;
          
          LEDData[0][3*i + 2] = ON;
          LEDData[1][3*i + 2] = ON;
          LEDData[2][3*i + 2] = ON;
          LEDData[3][3*i + 2] = ON;          
        }
        break;
    
  }
}

// Light up the LEDs of the two selected player
void DisplayTwoPlayerColor(LED_Color_t left, LED_Color_t right) {
    
  unsigned char i;
  
  if (left != RED && right != RED) {
      for (i=0; i<4; i++) {
        
        LEDData[0][3*i] = OFF;
        LEDData[1][3*i] = OFF; 
        LEDData[2][3*i] = OFF;
        LEDData[3][3*i] = OFF;         
        
        LEDData[0][3*i + 1] = ON;
        LEDData[1][3*i + 1] = ON;
        LEDData[2][3*i + 1] = ON;
        LEDData[3][3*i + 1] = ON;
        
        LEDData[0][3*i + 2] = ON;
        LEDData[1][3*i + 2] = ON;
        LEDData[2][3*i + 2] = ON;
        LEDData[3][3*i + 2] = ON;          
      }
  }
  else if (left != GREEN && right != GREEN) {
        for (i=0; i<4; i++) {
        
        LEDData[0][3*i] = ON;
        LEDData[1][3*i] = ON;
        LEDData[2][3*i] = ON;
        LEDData[3][3*i] = ON;           
        
        LEDData[0][3*i + 1] = OFF;
        LEDData[1][3*i + 1] = OFF; 
        LEDData[2][3*i + 1] = OFF;
        LEDData[3][3*i + 1] = OFF;
        
        LEDData[0][3*i + 2] = ON;
        LEDData[1][3*i + 2] = ON;
        LEDData[2][3*i + 2] = ON;
        LEDData[3][3*i + 2] = ON;          
      }
  }
  
  else if (left != BLUE && right != BLUE) {    
      for (i=0; i<4; i++) {
      
        LEDData[0][3*i] = ON;
        LEDData[1][3*i] = ON;
        LEDData[2][3*i] = ON;
        LEDData[3][3*i] = ON;           
        
        LEDData[0][3*i + 1] = ON;
        LEDData[1][3*i + 1] = ON; 
        LEDData[2][3*i + 1] = ON;
        LEDData[3][3*i + 1] = ON;
        
        LEDData[0][3*i + 2] = OFF;
        LEDData[1][3*i + 2] = OFF;
        LEDData[2][3*i + 2] = OFF;
        LEDData[3][3*i + 2] = OFF;          
      }
  }
}

void TurnOffLEDs(void){
  
  unsigned char i;
  for (i=0; i<NUM_LEDS; i++) {
    LEDData[0][i] = OFF;
    LEDData[1][i] = OFF;
    LEDData[2][i] = OFF;
    LEDData[3][i] = OFF;
  }
}

/*---------------------------- Test Harness -----------------------------*/

#ifdef LED_TEST
void main(void)
{
  LED_Color_t standings[4][4];
  
  TMRS12_Init(TMRS12_RATE_1MS);
  //Set ports to output
  DDRT |= BIT7HI; // data line
  DDRT |= BIT6HI; // storage line
  DDRM |= BIT0HI; // shift line
  InitializeLED();
  
  printf("LED Initialized\r\n");

  standings[0][0] = RED;
  standings[0][1] = BLUE;
  standings[0][2] = NONE;
  standings[0][3] = GREEN;
  
  standings[1][0] = GREEN;
  standings[1][1] = RED;
  standings[1][2] = BLUE;
  standings[1][3] = NONE;
  
  standings[2][0] = BLUE;
  standings[2][1] = NONE;
  standings[2][2] = RED;
  standings[2][3] = GREEN;
  
  standings[3][0] = NONE;
  standings[3][1] = RED;
  standings[3][2] = GREEN;
  standings[3][3] = BLUE;
  
  //UpdatePattern(standings);
  UpdateWinnerPattern(GREEN);
  //DisplayOnePlayerColor(2);
  //DisplayTwoPlayerColor(BLUE, RED);
  //TurnOffLEDs();
  
  while(1) {
    /*
    for (i=0; i<4; i++) {
      for (j=0; j<NUM_LEDS; j++) {
        printf("%d ", LEDData[i][j]);
      }
      printf("\r\n");
    }
    printf("\r\n");
    */
    
    //printf("%d\r\n", counter);
    CheckLEDEvents();
  }
    
}
#endif // LED_TEST

/*----------------------------- End of File -----------------------------*/