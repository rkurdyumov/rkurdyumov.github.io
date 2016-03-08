#ifndef GAME_H
#define GAME_H

/**************************************************************************

Game Module
 
**************************************************************************/


/*-------------------------- Public Variables ---------------------------*/
typedef enum {
  PLAYER_RED, 
  PLAYER_GREEN,
  PLAYER_BLUE,
  NO_PLAYER
}PLAYER_COLOR_t;

typedef enum {
  OVERDRAFT,
  VALID
}OVERDRAFT_t;


/*-------------------------- Public Functions ---------------------------*/
void StartGame(void);
OVERDRAFT_t CheckOverdraft(PLAYER_COLOR_t player, unsigned int amount);
void SendMoney(PLAYER_COLOR_t player, unsigned char region, unsigned int amount);
unsigned char UpdateWinner(void);
void DepositPlayerBank(void);
unsigned int GetPlayerBank(PLAYER_COLOR_t);
PLAYER_COLOR_t GetCurrentStandings(unsigned char region, unsigned char standing);

#endif // GAME_H