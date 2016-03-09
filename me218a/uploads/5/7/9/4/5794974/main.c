/**************************************************************************

Main Module
 
This module coordinates all the other modules.  It uses a state machine
architecture to respond to events.
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ME218_C32.h>
#include "ServoLib.h"
#include <timers12.h>
#include "ADS12.h"
#include "PWMS12.h"

// Module includes
#include "ballot_motor.h"
#include "ballot_sensor.h"
#include "bank_servo.h"
#include "card_sensor.h"
#include "clock_servo.h"
#include "fan.h"
#include "game.h"
#include "LED.h"
#include "pot.h"
#include "region_buttons.h"
#include "vibrate_motor.h"
/*--------------------------- Module Defines ----------------------------*/
#define INITIAL_BANK      2000
#define WINNER_BANK       3000

#define DEPOSIT_TIMER     1
#define GAME_TIMER        2
#define CLOCK_SERVO_TIMER 3
#define FAN_TIMER         4
#define BALLOT_TIMER      5
#define L_VIBRATE_TIMER   6
#define R_VIBRATE_TIMER   7

#define BALLOT_MOTOR_TIME  500
#define FAN_TIME           4000
#define OVERDRAFT_TIME     300
#define LEVER_RESET_TIME   100
#define SAME_CARD_VIB_TIME 500     
#define GAME_TIME          30000
#define SERVO_TIME         1000
#define DEPOSIT_TIME       5001

/*---------------------------- Module Types -----------------------------*/
typedef enum {
  PLAYING_GAME,
  GAME_OVER,
  WAITING_2ND_CARD,
  WAITING_FAN,
  WAITING_BALLOT
}GAME_STATE_t;

/*-------------------------- Module Variables ---------------------------*/
static GAME_STATE_t currGameState;
static PLAYER_COLOR_t leftPlayer, rightPlayer, computerPlayer;
static unsigned char leftLeverReset, rightLeverReset;
static unsigned char computerRegion1, computerRegion2;

/*-------------------------- Module Functions ---------------------------*/
static void InitializePorts(void);
static void InitializeGame(void);
static void CalibrateCardSensors(void);
static void CalculateComputerRegion(void);
static void ComputerAction(void);
static void HandleRegionButtonPress(unsigned char button);
static unsigned char CheckClockServoExpire(void);
static unsigned char CheckGameTimerExpire(void);
static unsigned char CheckDepositTimerExpire(void);
static unsigned char CheckFanTimerExpire(void);
static void CheckTimerExpires(void);
static void GoToGameOver(void);

/*----------------------------- Module Code -----------------------------*/

static void InitializePorts(void) {
  
  // Initializations
  // Port E automatically input (ballot sensor)
  
  // Port T:
  
  // Initialize Servo outputs 
  // T5: R bank motor
  // T4: L bank motor
  // T3: Game clock motor
  Servo12_Init("xxSSSxxx");
  
  // Initialize PWM outputs
  // T0: R Vibrate Motor
  // T1: L Vibrate Motor
  // T2: Ballot Motor
  PWMS12_Init();
  
  // T6: Shift Register Storage Clock
  // T7: Shift Register Data In
  DDRT |= BIT6HI;
  DDRT |= BIT7HI;
  
  // Port A:
  // A0-A1 : L, R potentiometer
  // A2-A3 : L, R card color sensor
  // A4-A5 : L, R fan
  // A6 :    FREE
  // A7 :    MUX SELECT
  ADS12_Init("OOOOAAAA");
  
  // Port M:
  DDRM |= BIT0HI; // Shift Register Shift Clock
  DDRM |= BIT1HI; // FREE
  DDRM &= BIT2LO; // Mux 1Y
  DDRM &= BIT3LO; // Mux 2Y
  DDRM &= BIT4LO; // Mux 3Y
  DDRM &= BIT5LO; // Mux 4Y
}

static void InitializeGame(void) {
  
  TMRS12_Init(TMRS12_RATE_1MS);
  
  InitializePorts();
  InitializeCardSensor();
  InitializeButtons();
  InitializeLED();
  UpdateWinnerPattern(RED);
  StopFan(BOTH_FANS); 
  StopBallotMotor();
  StopVibrateMotor(BOTH_MOTORS);
  SetBankLevel(0, 0);
  currGameState = GAME_OVER;
  leftPlayer = rightPlayer = computerPlayer = NO_PLAYER;
  leftLeverReset = rightLeverReset = 1;
}

static void CalibrateCardSensors(void) {
  
  // Calibrate sequence
  printf("Press button 1a to sample left no card\r\n");
  while(CheckButtonPress() != 1) {
  	SetCardCalibration(1);
  }
  printf("Press button 1b to sample left green\r\n");
  while(CheckButtonPress() != 2) {
  	SetCardCalibration(2);
  }
  printf("Press button 2a to sample left blue\r\n");
  while(CheckButtonPress() != 3) {
  	SetCardCalibration(3);
  }
  printf("Press button 2b to sample left red\r\n");
  while(CheckButtonPress() != 4) {
  	SetCardCalibration(4);
  }
  printf("Press button 4 to sample right no card\r\n");
  while(CheckButtonPress() != 5) {
  	SetCardCalibration(5);
  }
  printf("Press button 5 to sample right green\r\n");
  while(CheckButtonPress() != 6) {
  	SetCardCalibration(6);
  }
  printf("Press button 6 to sample right blue\r\n");
  while(CheckButtonPress() != 7) {
  	SetCardCalibration(7);
  }
  printf("Press button 7 to sample right red\r\n");
  while(CheckButtonPress() != 8) {
  	SetCardCalibration(8);
  }
  printf("Press button 0 to start game\r\n");
  while(CheckButtonPress() != 1) {
	// do nothing
  }
}

// Generate two adjacent random regions for computer to target
static void CalculateComputerRegion(void) {

  unsigned char n;
  n = (unsigned char)(rand()%4);	// generate random n from 0->3
  computerRegion1 = n; 
  computerRegion2 = (unsigned char)(n+1)%4;
}

// Computer strategy
void ComputerAction(void) {
  
  unsigned int computerMoney;
  LED_Color_t currStandings[4][4];
  unsigned char i,j;
  computerMoney = GetPlayerBank(computerPlayer); // extract computer money
  // send equal amounts to this game's random regions
  SendMoney(computerPlayer, computerRegion1, computerMoney/2);
  SendMoney(computerPlayer, computerRegion2, computerMoney/2);
  printf("Computer player %d sending %d to region %d\r\n", computerPlayer,computerMoney/2, computerRegion1);
  printf("Computer player %d sending %d to region %d\r\n", computerPlayer,computerMoney/2, computerRegion2);
  printf("Left bank: %d Right bank: %d Comp bank: %d\r\n", 
        GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer), GetPlayerBank(computerPlayer));
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      currStandings[i][j] = GetCurrentStandings(i,j);
    }
  }
  UpdatePattern(currStandings); // update LEDs
}

// Handle region button press
static void HandleRegionButtonPress(unsigned char button) {
  
  unsigned short money;
  unsigned char i,j;
  LED_Color_t currStandings[4][4];
  
  // if left player button pressed
  if (button >= 1 && button <= 4) {
    // if left lever reset
	if (leftLeverReset == 1) {
      money = ReturnMoney(LEFT_LEVER); // calculate money lever level
	  if (CheckOverdraft(leftPlayer, money) == OVERDRAFT) {
        printf("Left player overdraft\r\n");
        VibrateMotor(LEFT_MOTOR);
        TMRS12_InitTimer(L_VIBRATE_TIMER, OVERDRAFT_TIME);
      }
      // not overdraft, proceed
	  else {
        SendMoney(leftPlayer, button - 1, money);
        printf("Left player %d sending %d to region %d\r\n", leftPlayer,money, (button - 1)%4);
        printf("Left bank: %d Right bank: %d Comp bank: %d\r\n", 
        GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer), GetPlayerBank(computerPlayer));
        printf("Button Press, bank level L: %d R: %d", GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer));
        SetBankLevel(GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer)); // update bank servos
        for (i=0; i<4; i++) {
          for (j=0; j<4; j++) {
            currStandings[i][j] = GetCurrentStandings(i,j);
            printf("%d ",currStandings[i][j]);
          }
          printf("\r\n");
        }
        printf("\r\n");
        UpdatePattern(currStandings); // update LEDs
      }
      leftLeverReset = 0; // left lever used
    }
	// if left lever not reset
    else {
      VibrateMotor(LEFT_MOTOR);
      printf("Left lever not reset\r\n");
      TMRS12_InitTimer(L_VIBRATE_TIMER, LEVER_RESET_TIME);
    }
  }
  // if right player button pressed
  else if (button >= 5 && button <= 8) {
    // if right lever reset
	if (rightLeverReset ==1) {
      printf("Right transaction, lever reset\r\n");
      money = ReturnMoney(RIGHT_LEVER); // calculate money lever level
      if (CheckOverdraft(rightPlayer, money) == OVERDRAFT) {
        printf("Right player overdraft\r\n");
        VibrateMotor(RIGHT_MOTOR);
        TMRS12_InitTimer(R_VIBRATE_TIMER, OVERDRAFT_TIME);
      }
	  // not overdraft, proceed
      else {
        SendMoney(rightPlayer, (unsigned char)(button - 1)%4, money);
        printf("Right player %d sending %d to region %d\r\n", rightPlayer, money, (button - 1)%4);
        printf("Left bank: %d Right bank: %d, Comp bank: %d \r\n", 
        GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer), GetPlayerBank(computerPlayer)); 
        printf("Button Press, bank level L: %d R: %d", GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer));
        SetBankLevel(GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer)); 
        for (i=0; i<4; i++) {
          for (j=0; j<4; j++) {
            currStandings[i][j] = GetCurrentStandings(i,j); 
            printf("%d ", currStandings[i][j]);
          }
          printf("\r\n");
        }
        printf("\r\n");
        UpdatePattern(currStandings); // update LEDs
      }
      rightLeverReset = 0; // right lever used
    }
	// if right lever not reset
    else {
      VibrateMotor(RIGHT_MOTOR);  
      printf("Right lever state: %d\r\n", rightLeverReset);
      printf("Right lever not reset\r\n");
      TMRS12_InitTimer(R_VIBRATE_TIMER, LEVER_RESET_TIME); 
    }
  }
}

// Check if clock servo has expired
static unsigned char CheckClockServoExpire(void) {
  
  unsigned char expireStatus = 0;
  
  // 1s clock servo timer
  if (TMRS12_IsTimerExpired(CLOCK_SERVO_TIMER) == TMRS12_EXPIRED) {
     
    expireStatus = 1;
    TMRS12_ClearTimerExpired(CLOCK_SERVO_TIMER);
  }
  return expireStatus; 
}

// Check if game timer has expired
static unsigned char CheckGameTimerExpire(void) {
  
  unsigned char expireStatus = 0;
  
  // 30s game timer
  if (TMRS12_IsTimerExpired(GAME_TIMER) == TMRS12_EXPIRED) {
    
    expireStatus = 1;
    TMRS12_ClearTimerExpired(GAME_TIMER);
  }
  return expireStatus;
}

// Check if deposit timer has expired
static unsigned char CheckDepositTimerExpire(void) {
  
  unsigned char expireStatus = 0;
  
  // 5s deposit timer
  if (TMRS12_IsTimerExpired(DEPOSIT_TIMER) == TMRS12_EXPIRED) {
    
    expireStatus = 1;
    TMRS12_ClearTimerExpired(DEPOSIT_TIMER);
  }
  return expireStatus;
}

// Check if fan timer has expired
static unsigned char CheckFanTimerExpire(void) {
  
  unsigned char expireStatus = 0;
  
  if (TMRS12_IsTimerExpired(FAN_TIMER) == TMRS12_EXPIRED) {
    
    expireStatus = 1;
    TMRS12_ClearTimerExpired(FAN_TIMER);
  } 
  return expireStatus;
}

// Check external timer expirations: event handling doesn't depend on state
static void CheckTimerExpires(void) {
  
  // ballot motor
  if (TMRS12_IsTimerExpired(BALLOT_TIMER) == TMRS12_EXPIRED) {
    
    StopBallotMotor();
    TMRS12_ClearTimerExpired(BALLOT_TIMER);
  }
  
  // left vibrate motor
  if (TMRS12_IsTimerExpired(L_VIBRATE_TIMER) == TMRS12_EXPIRED) {
    
    StopVibrateMotor(LEFT_MOTOR);
    TMRS12_ClearTimerExpired(L_VIBRATE_TIMER);
  }
  
  // right vibrate motor
  if (TMRS12_IsTimerExpired(R_VIBRATE_TIMER) == TMRS12_EXPIRED) {
    
    StopVibrateMotor(RIGHT_MOTOR);
    TMRS12_ClearTimerExpired(R_VIBRATE_TIMER);
  }
}

// End game
static void GoToGameOver(void) {
   
   unsigned char winner;
   
   leftLeverReset = rightLeverReset = 1; // reset levers
   winner = UpdateWinner();
   UpdateWinnerPattern(winner); // update LED pattern
   if (leftPlayer == winner)
    SetBankLevel(WINNER_BANK, 0);
   else if (rightPlayer == winner)
    SetBankLevel(0, WINNER_BANK);
   else
    SetBankLevel(0, 0); // computer wins
   EjectBallot();
   TMRS12_InitTimer(BALLOT_TIMER, BALLOT_MOTOR_TIME); // start ballot motor timer
   StartFan(BOTH_FANS);
   TMRS12_InitTimer(FAN_TIMER, FAN_TIME); // start fan timer
   leftPlayer = rightPlayer = computerPlayer = NO_PLAYER; // reset player assignments
}

void main(void) {

  GAME_STATE_t nextGameState;
  CARD_STATE_t currLeftState, currRightState;
  unsigned char buttonEvent;
  unsigned char servoExpire, gameExpire, depositExpire, fanExpire;
  BALLOT_EVENT_t ballotState;
  unsigned char leftLeverFlag, rightLeverFlag;
  
  CalibrateCardSensors();
  InitializeGame();

  while(1)  {
    nextGameState = currGameState;
    CheckLEDEvents(); // update LED flashing
	// Check for events, timer expirations, and lever resets
    buttonEvent = CheckButtonPress();
    ballotState = CheckBallotState();
    currLeftState = CheckLeftCardState();
    currRightState = CheckRightCardState();
    CheckTimerExpires();
    gameExpire = CheckGameTimerExpire();
    servoExpire = CheckClockServoExpire();
    depositExpire = CheckDepositTimerExpire();
    fanExpire = CheckFanTimerExpire();
    leftLeverFlag = CheckLeverReset(LEFT_LEVER);
    rightLeverFlag = CheckLeverReset(RIGHT_LEVER);
    
    //printf("GameState: %d\r\n",currGameState);
    //printf("leftLever: %d rightLever: %d\r\n", leftLeverFlag, rightLeverFlag);
    //printf("playerL: %d playerR: %d playerComp: %d\r\n",leftPlayer, rightPlayer, computerPlayer);
    //printf("fanExpire: %d\r\n",fanExpire);
    
    switch(currGameState) {
      
      case GAME_OVER:
		// If the left player inserts a color card
        if (currLeftState != NO_CARD && currRightState == NO_CARD) {
          leftPlayer = currLeftState;
          DisplayOnePlayerColor(leftPlayer);
          printf("Got left player: %d, going to waiting for 2nd card\r\n", leftPlayer);
          nextGameState = WAITING_2ND_CARD;
          break;
        }
		// If the right player inserts a color card
        else if (currLeftState == NO_CARD && currRightState != NO_CARD) {
          rightPlayer = currRightState;
          DisplayOnePlayerColor(rightPlayer);
          printf("Got right player %d, going to waiting for 2nd card\r\n", rightPlayer);
          nextGameState = WAITING_2ND_CARD;
          break;
        }
		// Else, wait
        else {
          nextGameState = GAME_OVER;
          break; 
        }
       
          
      case WAITING_2ND_CARD:
        // If the right player inserts a card
        if (leftPlayer != NO_PLAYER && currRightState != NO_CARD) {
           printf("Inside waiting for second card\r\n");
		   // If the cards match, reject
           if(currLeftState == currRightState) {
            printf("Same card inserted left %d right %d, fans on\r\n", currLeftState, currRightState);
            VibrateMotor(BOTH_MOTORS);
            TMRS12_InitTimer(L_VIBRATE_TIMER, SAME_CARD_VIB_TIME);
            TMRS12_InitTimer(R_VIBRATE_TIMER, SAME_CARD_VIB_TIME);
            StartFan(BOTH_FANS);
            TMRS12_InitTimer(FAN_TIMER, FAN_TIME);
            leftPlayer = NO_PLAYER;
            rightPlayer = NO_PLAYER;
            UpdateWinnerPattern(RED);
            nextGameState = WAITING_FAN;
            break;
           }
		   // If the cards don't match, assign computer player and proceed
           else {
            rightPlayer = currRightState;
            DisplayTwoPlayerColor(leftPlayer, rightPlayer);
            if (leftPlayer != PLAYER_RED && rightPlayer != PLAYER_RED)
              computerPlayer = PLAYER_RED;
            else if (leftPlayer != PLAYER_GREEN && rightPlayer != PLAYER_GREEN)
              computerPlayer = PLAYER_GREEN;
            else 
              computerPlayer = PLAYER_BLUE;
            ResetClock();
            printf("2nd card inserted, left %d right %d, going to waiting ballot\r\n", leftPlayer, rightPlayer);
            nextGameState = WAITING_BALLOT;
            break;
           } 
            
        }
		// If the left player inserts a card
        else if (rightPlayer != NO_PLAYER && currLeftState != NO_CARD) {
          // If the cards match, reject
		  if(currRightState == currLeftState) {
            printf("Same card inserted left %d right %d, fans on\r\n", currLeftState, currRightState); 
            VibrateMotor(BOTH_MOTORS);
            TMRS12_InitTimer(L_VIBRATE_TIMER, SAME_CARD_VIB_TIME);
            TMRS12_InitTimer(R_VIBRATE_TIMER, SAME_CARD_VIB_TIME);
            StartFan(BOTH_FANS);
            TMRS12_InitTimer(FAN_TIMER, FAN_TIME);
            leftPlayer = NO_PLAYER;
            rightPlayer = NO_PLAYER;
            UpdateWinnerPattern(RED);
            nextGameState = WAITING_FAN;
            break;
           }
		   // If the cards don't match, assign computer player and proceed
           else {
            leftPlayer = currLeftState;
            DisplayTwoPlayerColor(leftPlayer, rightPlayer);
            if (leftPlayer != PLAYER_RED && rightPlayer != PLAYER_RED)
              computerPlayer = PLAYER_RED;
            else if (leftPlayer != PLAYER_GREEN && rightPlayer != PLAYER_GREEN)
              computerPlayer = PLAYER_GREEN;
            else 
              computerPlayer = PLAYER_BLUE;
            ResetClock();
            printf("2nd card inserted, left %d right %d, going to waiting ballot\r\n", leftPlayer, rightPlayer);
            nextGameState = WAITING_BALLOT;
            break;
           }
        }
		// Else, wait
        else {
          nextGameState = WAITING_2ND_CARD;
          break;
        }
        
      case WAITING_FAN:
        if (fanExpire == 1) {
          printf("Stopping fans, going to game over\r\n");
          StopFan(BOTH_FANS);
          nextGameState = GAME_OVER;
          break;          
        } 
        else {
          nextGameState = WAITING_FAN;
          break;
        }
      
      case WAITING_BALLOT: 
		// If ballot sensed, start game
        if(ballotState == BALLOT_SENSED) {
          TurnOffLEDs();
          TMRS12_InitTimer(GAME_TIMER, GAME_TIME); // start game clock
          TMRS12_InitTimer(CLOCK_SERVO_TIMER, SERVO_TIME); // start servo clock
          TMRS12_InitTimer(DEPOSIT_TIMER, DEPOSIT_TIME); // deposit money timer
          SetBankLevel(INITIAL_BANK, INITIAL_BANK);
          StartGame();
          CalculateComputerRegion();
          printf("Ballot sensed, going to playing game\r\n");
          nextGameState = PLAYING_GAME;
          break;
        }
		// Else, wait
        else {
          nextGameState = WAITING_BALLOT;
          break;
        }
        
      case PLAYING_GAME:
        // If region button pressed, handle event
        if (buttonEvent != 0) {
          HandleRegionButtonPress(buttonEvent);
          nextGameState = PLAYING_GAME;
          break;
        }
		// If game expired, go to game over sequence
        if (gameExpire == 1) {
          TMRS12_StopTimer(DEPOSIT_TIMER); 
          TMRS12_StopTimer(CLOCK_SERVO_TIMER);
          TMRS12_ClearTimerExpired(DEPOSIT_TIMER);
          TMRS12_ClearTimerExpired(CLOCK_SERVO_TIMER);
          GoToGameOver();
          printf("Game timer expired, turning on fans, going to game over\r\n");
          nextGameState = WAITING_FAN;
          break;
        }
		// If clock servo timer expires, advance clock
        if (servoExpire == 1) {
          AdvanceClock();
          TMRS12_InitTimer(CLOCK_SERVO_TIMER, SERVO_TIME); // start servo clock again
          //printf("Servo advance\r\n");
          nextGameState = PLAYING_GAME;          
          break;
        }
		// If deposit timer expires, deposit money in player accounts
        if (depositExpire == 1) {
          DepositPlayerBank();
          // Computer deposit
          ComputerAction();
          SetBankLevel(GetPlayerBank(leftPlayer), GetPlayerBank(rightPlayer));
          TMRS12_InitTimer(DEPOSIT_TIMER, DEPOSIT_TIME);
          nextGameState = PLAYING_GAME;
          break;
        }
		// If lever(s) reset, then perform reset
        if (leftLeverFlag == 1 || rightLeverFlag ==1) {
          if (leftLeverFlag ==1)
          	leftLeverReset = 1;
          if (rightLeverFlag == 1)
          	rightLeverReset = 1;
          nextGameState = PLAYING_GAME;
          break;
        }
    }
	// Update the game state
    currGameState = nextGameState;
  }
}