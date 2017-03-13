#include "../common/utils.h"
#include "fsm.h"
#include "server.h"

void *print_message_function( void *ptr );

PlayerList* listOfPlayers = NULL;
int nbOfPlayers = 0;
int nbOfVictories = 0;
int nbOfDefeats = 0;
int sentRoles = 0;
int receivedVotes = 0;
int teamVoteResult = -1;
int turn = 0;
int tokenPosition = 0;

int main(int argc, char *argv[]) {
    pthread_t threadServer, threadFsm;
    int iret1, iret2;

    /* Create independent threads each of which will execute function */
    /* get a transportable copy of the command line arguments */
    MainArgs* args = (MainArgs*)malloc(sizeof(MainArgs));
    args->argc = argc;
    args->argv = (char**)malloc(argc * sizeof(char*));
    int i;
    for(i = 0; i < argc; i++)
        args->argv[i] = strdup(argv[i]);

    /* create server thread */
    iret1 = pthread_create(&threadServer, NULL, runServer, args);
    if(iret1) {
        fprintf(stderr, "Error - pthread_create() return code: %d\n", iret1);
        exit(EXIT_FAILURE);
    }

    /* create FSM thread */
    iret2 = pthread_create(&threadFsm, NULL, runFSM, NULL);
    if(iret2) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", iret2);
        exit(EXIT_FAILURE);
    }

    printf("%s>> pthread_create() for server thread returns: %d\n", COLOR_YEL, iret1);
    printf(">> pthread_create() for FSM thread returns: %d%s\n", iret2, COLOR_NRM);

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

    //pthread_join(threadServer, NULL);
    pthread_join(threadFsm, NULL);

    printf("%s>> exiting threads%s\n", COLOR_YEL, COLOR_NRM);

    exit(EXIT_SUCCESS);
}

void *print_message_function(void *ptr) {
    char *message;
    message = (char *) ptr;
    printf("%s \n", message);
}

