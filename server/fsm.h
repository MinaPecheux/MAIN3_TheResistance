/* Finite State Machine using C Function Pointers.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Header file.
*/

#ifndef __FSM_H__
#define __FSM_H__

#include <time.h>

void waitingForPlayers();
void gettingRandomRoles();
void sendingRandomRoles();
void proposingTeam();
void pickingTeam();
void completingMission();

void* runFSM();

extern int running;
extern int nbOfVictories;
extern int nbOfDefeats;

#endif
