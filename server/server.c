/* Server manager. Uses TCP connection.
    The port number is passed as an argument.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Script file.
*/

#include "../common/utils.h"
#include "server.h"
#include "fsm.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void* runServer(MainArgs* args) {
    int argc = args->argc;
    char** argv = args->argv;
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    	error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
    				 sizeof(serv_addr)) < 0)
    	error("ERROR on binding");
    listen(sockfd, MAX_NB_OF_PLAYERS);
    clilen = sizeof(cli_addr);

    while(1) {
        /* start of the protected section */
        pthread_mutex_lock(&mutex_vars);

        /* routine depending on the current FSM state */
        if(currentState == WAITING_FOR_PLAYERS) {
            while (nbOfPlayers < MAX_NB_OF_PLAYERS) {
                newsockfd = accept(sockfd, 
                             (struct sockaddr *) &cli_addr, 
                             &clilen);
                if (newsockfd < 0) error("ERROR on accept");

                bzero(buffer, 256);
                n = read(newsockfd, buffer, 255);
                if (n < 0) error("ERROR reading from socket");

                /* decode read buffer */
                char playerName[256];
                strcpy(playerName, buffer);

                /* if not already connected a player with this name: add player to the players list */
                if(findPlayerAtIndex(listOfPlayers, playerName) != NULL) {
                    printf("%s<<< connecting player: %s%s\n", COLOR_BLU, playerName, COLOR_NRM);
                    connectPlayer(playerName);
                }
                //if(nbOfPlayers == MAX_NB_OF_PLAYERS)
                //    pthread_cond_signal(&cond_connectedPlayers);

                shutdown(newsockfd, SHUT_RDWR);
            }
        }
        else if(currentState == GETTING_RANDOM_ROLES) {}
        else if(currentState == SENDING_RANDOM_ROLES) {
            while(sentRoles < MAX_NB_OF_PLAYERS) {
                printf("%s<<< sending%s\n", COLOR_BLU, COLOR_NRM);
                int i;
                for(i = 0; i < MAX_NB_OF_PLAYERS; i++) {
                    PlayerList* p = findPlayerAtIndex(listOfPlayers, i);
                    printf("- %s, you are a %s.\n", p->name, roles[p->isSpy]);
                    sentRoles++;
                }
            }
        }
        else if(currentState == PROPOSING_TEAM) {}
        else if(currentState == PICKING_TEAM) {
            printf("%s<<< picking%s\n", COLOR_BLU, COLOR_NRM);
            /* give some time to the FSM to prepare the team */
            sleep(1);

            int vote = -1;
            char playerName[256];
            char voteChar[1];
            VoterList* voterList = NULL;
            /* do the vote */
            while (receivedVotes < MAX_NB_OF_PLAYERS) {
                newsockfd = accept(sockfd, 
                             (struct sockaddr *) &cli_addr, 
                             &clilen);
                if (newsockfd < 0) error("ERROR on accept");

                bzero(buffer, 256);
                n = read(newsockfd, buffer, 255);
                if (n < 0) error("ERROR reading from socket");

                /* decode read buffer */
                sscanf(buffer, "%s %s", playerName, voteChar);
                if(strcmp(voteChar, "y") == 0) vote = 1;
                else if(strcmp(voteChar, "n") == 0) vote = 0;
                else vote = -1;

                /* if the player has not already voted, add his vote to the list */
                if(!alreadyVoted(voterList, playerName)) {
                    voterList = addVoter(voterList, playerName, vote);
                    receivedVotes++;
                    printf("%s<<< player %s voted %s%s\n", COLOR_BLU, playerName, voteChar, COLOR_NRM);
                } else {
                    printf("%s<<< player %s already voted!%s\n", COLOR_BLU, playerName, COLOR_NRM);
                }

                shutdown(newsockfd, SHUT_RDWR);
            }
            teamVoteResult = getVotingResult(voterList);
            destroyList(voterList);
        }
        else if(currentState == COMPLETING_MISSION) {}
        else {
            printf("%s<<< default case%s\n", COLOR_BLU, COLOR_NRM);
            if(!running)
                break;
        }

        /* start of the protected section */
        pthread_mutex_unlock(&mutex_vars);
    }

 	/*newsockfd = accept(sockfd, 
             (struct sockaddr *) &cli_addr, 
             &clilen);
 	if (newsockfd < 0) 
      	error("ERROR on accept");

 	bzero(buffer, 256);
 	n = read(newsockfd, buffer, 255);
 	if (n < 0) 
	error("ERROR reading from socket");

    printf("%s\n", buffer);

 	close(newsockfd);*/

    close(sockfd);

    return NULL;
}
