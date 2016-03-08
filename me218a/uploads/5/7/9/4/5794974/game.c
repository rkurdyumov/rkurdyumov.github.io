//#define GAME_TEST
/**************************************************************************

Game Module
 
**************************************************************************/

/*---------------------------- Include Files ----------------------------*/
#include <stdio.h>
#include <ME218_C32.h>
#include <timers12.h>
#include "ADS12.h"
#include "game.h"
/*--------------------------- Module Defines ----------------------------*/
/*---------------------------- Module Types -----------------------------*/
/*-------------------------- Module Variables ---------------------------*/
static unsigned int currentRegionBank[4][3];
static unsigned int moneyRates[4][3];
static unsigned int regionValue[4];
static unsigned int playerDeposit[3];
static PLAYER_COLOR_t currentStandings[4][4];
static unsigned int currentPlayerBank[3];

/*-------------------------- Module Functions ---------------------------*/
static void UpdateStandings(unsigned char region);

/*----------------------------- Module Code -----------------------------*/

// Return amount in player's bank
unsigned int GetPlayerBank(PLAYER_COLOR_t player) {

  unsigned char i;
  unsigned int amount;
  
  for(i=0; i<3; i++) { 
    if(player == i) {
      amount = currentPlayerBank[i];
    }
  }
  return amount;
}

// Return current region standing
PLAYER_COLOR_t GetCurrentStandings(unsigned char region, unsigned char standing) {
  
  unsigned char i, j;
  PLAYER_COLOR_t player;
  
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      if (i==region && j == standing)
        player = currentStandings[i][j]; 
    }
  }
  return player;
}

// Check if player is overdrafting
OVERDRAFT_t CheckOverdraft(PLAYER_COLOR_t player, unsigned int amount) {
  
  if (currentPlayerBank[player] < amount)
    return OVERDRAFT;
  else
    return VALID;
}

// Send money amount by a player to a region
void SendMoney(PLAYER_COLOR_t player, unsigned char region, unsigned int amount) {
  
  currentPlayerBank[player] -= amount;
  // Send amount based on effectiveness rate for that region/player combination
  currentRegionBank[region][player] += amount * moneyRates[region][player] / 12;
  UpdateStandings(region);
  UpdateWinner();
}

// Make a deposit to the player's bank
void DepositPlayerBank(void) {
  
  unsigned char i;
  
  for (i=0; i<3; i++) {
     currentPlayerBank[i] += playerDeposit[i];
  }
}

// Update the current standings in a region
static void UpdateStandings(unsigned char region) {

  unsigned char i;
  unsigned char max1, max2, max3;
  unsigned char counter1, counter2, counter3;
  counter1 = counter2 = counter3 = 0;
  
  // Find the player with the highest amount
  for (i=0; i<3; i++) {
    if (currentRegionBank[region][i] >= currentRegionBank[region][(i+1)%3] &&
    currentRegionBank[region][i] >= currentRegionBank[region][(i+2)%3] && 
    currentRegionBank[region][i] > 0) {
      counter1++;
      max1 = i;
    }
  }
  // If no player has deposited, there is no 1st place, stop
  if (counter1 == 0) {
    currentStandings[region][0] = NO_PLAYER;
    currentStandings[region][1] = NO_PLAYER;
    currentStandings[region][2] = NO_PLAYER;
    currentStandings[region][3] = NO_PLAYER;
    return;
  }
  
  // Find the player with the second highest amount
  for (i=0; i<3; i++) {
    if (i!=max1) {  
      if ((currentRegionBank[region][i] >= currentRegionBank[region][(i+1)%3] ||
      currentRegionBank[region][i] >= currentRegionBank[region][(i+2)%3]) && 
      currentRegionBank[region][i] > 0) {
        counter2++;
        max2 = i;
      }
    }
  }
  // If only one player has deposited, there is no 2nd place, set 1st place, stop
  if (counter2 == 0) {
    currentStandings[region][0] = max1;
    currentStandings[region][1] = NO_PLAYER;
    currentStandings[region][2] = NO_PLAYER;
    currentStandings[region][3] = NO_PLAYER;
    return;
  }
  
  // Find the player with the third highest amount
  for (i=0; i<3; i++) {
    if (i!=max1 && i!= max2) {  
      if (currentRegionBank[region][i] > 0) {
        counter3++;
        max3 = i;
      }
    }
  }
  // If only two players have deposited, there is no 3rd place, set 1st/2nd place, stop
  if (counter3 == 0) {
    currentStandings[region][0] = max1;
    currentStandings[region][1] = max2;
    currentStandings[region][2] = NO_PLAYER;
    currentStandings[region][3] = NO_PLAYER;
    return;
  }
  
  // Otherwise, set 1st/2nd/3rd place, stop
  currentStandings[region][0] = max1;
  currentStandings[region][1] = max2;
  currentStandings[region][2] = max3;
  currentStandings[region][3] = NO_PLAYER;
  return;
}

// Update the current winner
unsigned char UpdateWinner(void) {
  
  unsigned char currentWinner;
  unsigned int total[3] = {0, 0, 0};
  unsigned char i, j;
  
  // Calculate totals
  for (i=0; i<3; i++) {
    for (j=0; j<4; j++) {
      if (currentStandings[j][0] == i)
        total[i] += regionValue[j];
    }
  }
  
  // Check for the highest total
  for (i=0; i<3; i++) {
    if (total[i] >= total[(i+1)%3] && total[i] >= total[(i+2)%3]) {
      currentWinner = i;
    }
  }
  return currentWinner;
}

// Initialize the game
void StartGame(void) {
  
  unsigned char i, j;
  
  // Set initial bank
  for(i=0; i<3; i++)
    currentPlayerBank[i] = 2000;
  
  // Initialize initial player standings and bank in each region
  for(i=0; i<4; i++) {
    for(j=0; j<4; j++) {
      currentRegionBank[i][j] = 0;
      currentStandings[i][j] = NO_PLAYER;  
    }
  }
  
  // Set value for each region
  regionValue[0] = 10;
  regionValue[1] = 10;
  regionValue[2] = 10;
  regionValue[3] = 10;
  
  // Set periodic deposit amount for each player
  playerDeposit[PLAYER_BLUE] = 400;
  playerDeposit[PLAYER_GREEN] = 300;
  playerDeposit[PLAYER_RED] = 200;
 
  // Set money rates for each region based on player
  // Equal
  moneyRates[0][PLAYER_BLUE] = 12;
  moneyRates[0][PLAYER_GREEN] = 9;
  moneyRates[0][PLAYER_RED] = 6;
  
  // Red advantage
  moneyRates[1][PLAYER_BLUE] = 12;
  moneyRates[1][PLAYER_GREEN] = 9;
  moneyRates[1][PLAYER_RED] = 7;
  
  // Green advantage
  moneyRates[2][PLAYER_BLUE] = 12;
  moneyRates[2][PLAYER_GREEN] = 10;
  moneyRates[2][PLAYER_RED] = 6;
  
  // Blue advantage
  moneyRates[3][PLAYER_BLUE] = 13;
  moneyRates[3][PLAYER_GREEN] = 9;
  moneyRates[3][PLAYER_RED] = 6;
}

#ifdef GAME_TEST
 
 void main(void) {
  
  unsigned char i, j;
  unsigned char currentWinner;
  
  StartGame();
  SendMoney(PLAYER_RED, 2, 256);
  SendMoney(PLAYER_GREEN, 0, 451);
  SendMoney(PLAYER_BLUE, 2, 56);
  SendMoney(PLAYER_RED, 1, 645);
  SendMoney(PLAYER_GREEN, 3, 200);
  SendMoney(PLAYER_BLUE, 2, 500);
  DepositPlayerBank();
  currentWinner = UpdateWinner();
  
  
  printf("Current standings\r\n");
  
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      printf("%d ", currentStandings[i][j]);
    }
    printf("\r\n");
  }
  printf("\r\n");
  
  printf("Current Region Bank\r\n");
  for (i=0; i<4; i++) {
    for (j=0; j<3; j++) {
      printf("%d ", currentRegionBank[i][j]);
    }
    printf("\r\n");
  }
  printf("\r\n");
  
  printf("Current Player Bank\r\n");
  for (i=0; i<3; i++) {
    printf("%d ", currentPlayerBank[i]);
  }
  printf("\r\n");
  
  printf("Money rates\r\n");
  for (i=0; i<4; i++) {
    for (j=0; j<3; j++) {
      printf("%d ", moneyRates[i][j]);
    }
    printf("\r\n");
  }
  printf("\r\n");
  
  printf("Current winner: %d\r\n", currentWinner);
  
  printf("Testing Get Player Bank\r\n");
  printf("%d %d %d\r\n", GetPlayerBank(0), GetPlayerBank(1), GetPlayerBank(2));
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      printf("%d ", GetCurrentStandings(i,j));
    }
    printf("\r\n");
  }
  printf("\r\n");
  
 }
#endif