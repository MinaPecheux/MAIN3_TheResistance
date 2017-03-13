/* Useful structs and functions for the project.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Script file.
*/

#include "utils.h"

/* GENERAL UTILS FUNCTIONS */
int intInArray(int* array, int arraySize, int value) {
	int i;
	for(i = 0; i < arraySize; i++) {
		if(array[i] == value) return 1;
	}

	return 0;
}

void connectPlayer(char* playerName) {
  //printf("Connecting Player #%d: %s\n", nbOfPlayers, playerName);
  listOfPlayers = addPlayer(listOfPlayers, nbOfPlayers, playerName);
  nbOfPlayers++;
}

/* PLAYER LIST FUNCTIONS */
PlayerList* addPlayer(PlayerList* playerList, int uid, char* name) {
	/* create new player instance */
	PlayerList* p = (PlayerList*)malloc(sizeof(PlayerList));
	p->uid = uid;
	p->name = strdup(name);
	p->isSpy = 0;

	/* add it in front of the list */
	p->next = playerList;

	return p;
}

PlayerList* findPlayerAtIndex(PlayerList* playerList, int index) {
	PlayerList* tmp = playerList;
	int count = 0;
	while(tmp != NULL) {
		if(count == index)
			return tmp;
		count++;
		tmp = tmp->next;
	}

	return NULL;	
}

void printPlayerList(PlayerList* playerList) {
	printf("-----------------------\nLIST OF PLAYERS:\n");
	PlayerList* tmp = playerList;
	while(tmp != NULL) {
		if(tmp->isSpy)
			printf("Player #%d: %s is a SPY\n", tmp->uid, tmp->name);
		else
			printf("Player #%d: %s is a REBEL\n", tmp->uid, tmp->name);
		tmp = tmp->next;
	}
	printf("-----------------------\n");
}

void destroyPlayerList(PlayerList* list) {
	PlayerList* tmp;
	while(list != NULL) {
		tmp = list;
		list = tmp->next;
		free(tmp->name);
		free(tmp);
	}
}

/* VOTER LIST FUNCTIONS */

/* function that adds a voter */
VoterList* addVoter(VoterList* list, char* name, int vote) {
	VoterList* newVoter = (VoterList*)malloc(sizeof(VoterList));
	newVoter->name = strdup(name);
	newVoter->vote = vote;
	newVoter->next = list;

	return newVoter;
}

/* function that checks if a voter has already voted! */
int alreadyVoted(VoterList* list, char* voterName) {
	VoterList* cur = list;
	while(cur != NULL) {
		if(strcmp(voterName, cur->name) == 0)
			return 1;
		cur = cur->next;
	}

	return 0;
}

/* function that prints the result of the voting */
int getVotingResult(VoterList* list) {
	int voteAll = 0, voteY = 0, voteN = 0;
	VoterList* cur = list;
	while(cur != NULL) {
		if(cur->vote == 1) voteY++;
		else if(cur->vote == 0) voteN++;
		voteAll++;
		cur = cur->next;
	}

	return voteY > voteN;
}

/* function that frees the allocated memory for the list */
void destroyList(VoterList* list) {
	VoterList* tmp;
	while(list != NULL) {
		tmp = list;
		list = tmp->next;
		free(tmp);
	}
}

