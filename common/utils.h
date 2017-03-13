/* Useful structs and functions for the project.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Header file.
*/

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAX_NB_OF_PLAYERS 3
#define NB_OF_SPIES 2
#define END_GAME_COUNT 3
#define MAX_NB_OF_TURNS 5

#define COLOR_YEL  "\x1B[33m"
#define COLOR_BLU  "\x1B[34m"
#define COLOR_NRM  "\x1B[0m"

static const char *roles[] = {"Rebel", "Spy"};
static const int teamSizes[] = {2, 3, 3, 2, 3};

typedef struct _playerList {
  int uid;
  char* name;
  int isSpy;
  struct _playerList* next;
} PlayerList;

typedef struct _voterList {
	char* name;
	int vote;
	struct _voterList* next;
} VoterList;

typedef struct _mainArgs {
    int argc;
    char** argv;
} MainArgs;

typedef enum _state {
  WAITING_FOR_PLAYERS,
  GETTING_RANDOM_ROLES,
  SENDING_RANDOM_ROLES,
  PROPOSING_TEAM,
  PICKING_TEAM,
  COMPLETING_MISSION
} State;

int intInArray(int* array, int arraySize, int value);
void connectPlayer(char* playerName);

PlayerList* addPlayer(PlayerList* playerList, int uid, char* name);
PlayerList* findPlayerAtIndex(PlayerList* playerList, int index);
void printPlayerList(PlayerList* list);
void destroyPlayerList(PlayerList* list);

VoterList* addVoter(VoterList* list, char* name, int vote);
int alreadyVoted(VoterList* list, char* voterName);
int getVotingResult(VoterList* list);
void destroyList(VoterList* list);

extern PlayerList* listOfPlayers;
extern int nbOfPlayers;
extern int sentRoles;
extern int receivedVotes;
extern int teamVoteResult;
extern int turn;
extern int tokenPosition;

extern State currentState;

static pthread_mutex_t mutex_vars;

/*static pthread_cond_t cond_connectedPlayers;
static pthread_cond_t cond_sentPlayersRoles;
static pthread_cond_t cond_receivedVotes;*/

#endif
