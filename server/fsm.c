/* Finite State Machine using C Function Pointers.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Script file.
*/

#include "fsm.h"
#include "../common/utils.h"

/* state pointer */
void (*statefunc)() = waitingForPlayers;

/* state routines definitions */
void waitingForPlayers() {
  printf("Waiting for players... (%d in the room)\n", nbOfPlayers);

  /* condition wait: */
  /* if enough players are connected: jump to next state */
  /*pthread_cond_wait(&cond_connectedPlayers, &mutex_vars);
  statefunc = gettingRandomRoles;
  currentState = GETTING_RANDOM_ROLES;*/
  if(nbOfPlayers == MAX_NB_OF_PLAYERS) {
      statefunc = gettingRandomRoles;
      currentState = GETTING_RANDOM_ROLES;
  }
}

void gettingRandomRoles() {
  printf("\nGetting random roles.\n");

  /* determine random positions for the spies */
  /* initialize spy indices array to -1 */
  int* spyPositions = (int*)malloc(NB_OF_SPIES * sizeof(int));
  int i;
  for(i = 0; i < NB_OF_SPIES; i++) {
    spyPositions[i] = -1;
  }
  /* get unique random numbers in the nb of players range */
  for(i = 0; i < NB_OF_SPIES; i++) {
    int pos;
    do {
      pos = rand() % MAX_NB_OF_PLAYERS;
    } while(intInArray(spyPositions, NB_OF_SPIES, pos));
    spyPositions[i] = pos;
  }
  /* assign spy roles */
  for(i = 0; i < NB_OF_SPIES; i++) {
    findPlayerAtIndex(listOfPlayers, spyPositions[i])->isSpy = 1;
  }

  /* free allocated array */
  free(spyPositions);

  //printPlayerList(listOfPlayers);

  /* jump to next state */
  statefunc = sendingRandomRoles;
  currentState = SENDING_RANDOM_ROLES;
}

void sendingRandomRoles() {
  printf("\nSending random roles to players.\n");

  /* if all roles have been sent: jump to next state */
  if(sentRoles == MAX_NB_OF_PLAYERS) {
    statefunc = proposingTeam;
    currentState = PROPOSING_TEAM;
  }
}

void proposingTeam() {
  printf("\n------ TURN %d ------\n", turn + 1);
  printf("Proposing team.\n");

  /* determine random positions for the spies */
  /* initialize spy indices array to -1 */
  int* selectedPlayers = (int*)malloc(NB_OF_SPIES * sizeof(int));
  int i;
  for(i = 0; i < teamSizes[turn]; i++) {
    selectedPlayers[i] = -1;
  }
  /* get unique random numbers in the nb of players range */
  for(i = 0; i < teamSizes[turn]; i++) {
    int pos;
    do {
      pos = rand() % MAX_NB_OF_PLAYERS;
    } while(intInArray(selectedPlayers, teamSizes[turn], pos));
    selectedPlayers[i] = pos;
  }

  /* display result */
  printf("%s picks %d players:\n", findPlayerAtIndex(listOfPlayers, tokenPosition)->name, teamSizes[turn]);
  for(i = 0; i < teamSizes[turn]; i++) {
    printf("- %s\n", findPlayerAtIndex(listOfPlayers, selectedPlayers[i])->name);
  }
  printf("Do you agree with this team? (y/n)\n");

  /* free allocated array */
  free(selectedPlayers);

  statefunc = pickingTeam;
  currentState = PICKING_TEAM;
}

void pickingTeam() {
  /* when we get the vote result (i.e. everybody voted): */
  if(receivedVotes == MAX_NB_OF_PLAYERS) {
    /* if the team is OK, we proceed to next step */
    if(teamVoteResult == 1) {
      statefunc = completingMission;
      currentState = COMPLETING_MISSION;
    }
    /* else we pass the token and we redo a vote */
    else {
      /* pass the token to the next player */
      if(tokenPosition < MAX_NB_OF_PLAYERS - 1)
        tokenPosition++;
      else
        tokenPosition = 0;

      /* reinitialize vote counters */
      receivedVotes = 0;
      teamVoteResult = -1;

      /* go back to 'proposing team' state */
      statefunc = proposingTeam;
      currentState = PROPOSING_TEAM;
    }
  }
}

void completingMission() {
  printf("\nCompleting mission...\n");

  int v = rand() % 2;
  if(v) {
    printf("Mission result: VICTORY!\n");
    nbOfVictories++;
  }
  else {
    printf("Mission result: DEFEAT!\n");
    nbOfDefeats++;
  }

  /* new turn */
  turn++;

  /* check if the game has ended */
  if(nbOfVictories == END_GAME_COUNT || nbOfDefeats == END_GAME_COUNT || turn == MAX_NB_OF_TURNS)
    running = 0;
  else {
    /* pass the token to the next player */
    if(tokenPosition < MAX_NB_OF_PLAYERS - 1)
      tokenPosition++;
    else
      tokenPosition = 0;

    /* reinitialize vote counters */
    receivedVotes = 0;
    teamVoteResult = -1;

    /* go back to 'proposing team' state */
    statefunc = proposingTeam;
    currentState = PROPOSING_TEAM;
  }
}

int running = 1;
State currentState = WAITING_FOR_PLAYERS;

void* runFSM() {
  /* initialize random seed */
  srand(time(NULL));

  /* endless loop for the server */
  do {
    /* start of the protected section */
    pthread_mutex_lock(&mutex_vars);

    (*statefunc)();
    sleep(1); /* sleep for a second */

    /* end of the protected section */
    pthread_mutex_unlock(&mutex_vars);
  } while(running);

  /* end game display */
  if(nbOfVictories == END_GAME_COUNT)
    printf("\n---------------\nREBELS WIN!\n");
  else
    printf("\n---------------\nSPIES WIN!\n");

  destroyPlayerList(listOfPlayers);

  return NULL;
}
