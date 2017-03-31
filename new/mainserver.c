#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#include <pthread.h>

#define END 3

int portno;
int nbj;  // compteur joueur == nbj pour commencer
int nbespions;
int meneurCourant = 0;
int sameVote = 0;   // pour verifier si le jeton de meneur a fait un tour complet pour un meme vote
int compteurJoueurs; // pour compter les joueurs
int compteurMissions = 0; // pour savoir à quelle mission on est
int compteurVictoires = 0; // pour savoir combien de missions ont ete "succes"
int compteurDefaites = 0; // pour savoir combien de missions ont ete "echec"
int compteurVotes; // combien de votes ont été éffectués
int compteurReussites; // combien de vote reussite
int compteurEchecs;		// combien de vote echec
int compteurRebelles; // combien de rebelles
int compteurEspions;  // combien d'Espions
int participantsMissions[5]={2,3,2,3,3};	// pour savoir combien de participant à la mission
int nbparticipants = 1;
char serverbuffer[256];

char com;
char adrip[20];
int dportno;
char name[20];
char team[10];
//int equipe[10];

typedef enum {
    WAITING_FOR_PLAYERS,
    SENDING_ROLES,
    PROPOSING_TEAM,
    VOTING_TEAM,
    COMPLETING_MISSION,
    ENDING_GAME
} State;

State currentState = WAITING_FOR_PLAYERS;

/* codes des messages :
C -> connexion
m -> message pour le text_view
M -> infos sur la mission
P -> proposition d'equipe
V -> vote
L -> lancer la mission
N -> envoi des comptes de missions reussies/ratees
R -> resultat de la partie
*/

/* PRINCIPE :
		attente 'C'
		remplir la structure
		renvoyer le joueur aux autres joueurs
		si compteur == nbj
		fsmstate = 101 ;

*/
struct joueur
{
	char nom[20];
	char ipaddress[20];
	int portno;
	int equipe; // 0=pas dans equipe, 1=dans equipe
	int role; // 0=rebelle, 1=espion
	int vote; // 0=refus, 1=accept
	int reussite; // 0=echec, 1=reussite
} tableauJoueurs[5];
int roles[5];

void sendMessage(int j, char *mess);
void broadcast(char *message);
void sendRoles();
void sendMeneur();
void sendEquipe();
void sendChosenOnes();

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int intInArray(int* array, int arraySize, int value) {
    int i;
    for(i = 0; i < arraySize; i++) {
        if(array[i] == value) return 1;
    }

    return 0;
}

int findPlayerId(char* nom) {
    int i;
    for(i = 0; i < compteurJoueurs; i++) {
        if(strcmp(tableauJoueurs[i].nom, nom) == 0)
            return i;
    }
    return -1;
}

void initRoles() {
    /* determine random positions for the spies */
    /* initialize spy indices array to -1 */
    int* posEspions = (int*)malloc(nbespions * sizeof(int));
    int i;
    for(i = 0; i < nbespions; i++)
        posEspions[i] = -1;
    /* get unique random numbers in the nb of players range */
    for(i = 0; i < nbespions; i++) {
        int pos;
        do {
            pos = rand() % nbj;
        } while(intInArray(posEspions, nbespions, pos));
        posEspions[i] = pos;
    }
    /* assign spy roles */
    for(i = 0; i < nbj; i++) {
        if(intInArray(posEspions, nbespions, i)) {
            tableauJoueurs[i].role = 1;
        }
        else
            tableauJoueurs[i].role = 0;
    }

    /* free allocated array */
    free(posEspions);
}

void switchCompleteMission() {
    if(compteurMissions == 5 || compteurVictoires == END || compteurDefaites == END)
    	currentState = ENDING_GAME;
    else
    	currentState = PROPOSING_TEAM;
}

void *server(void *ptr)
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0)
            error("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    int running = 1;
    while (running)
    {
        char mess[100];
        switch(currentState) {
            case WAITING_FOR_PLAYERS:
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0)
                    error("ERROR on accept");
                bzero(serverbuffer, 256);
                n = read(newsockfd, serverbuffer, 255);
                if (n < 0) error("ERROR reading from socket");
                printf("Here is a message from a client: '%s' '%c'\n", serverbuffer, serverbuffer[0]);

                if (serverbuffer[0] == 'C') {
                    char connect;
                    bzero(mess, 100);

                    printf("Commande C\n");
                    sscanf(serverbuffer, "%c %s %d %s " , &connect,
                        tableauJoueurs[compteurJoueurs].ipaddress, 
                        &tableauJoueurs[compteurJoueurs].portno, 
                        tableauJoueurs[compteurJoueurs].nom);
                    sprintf(mess, "C %s %d", tableauJoueurs[compteurJoueurs].nom, compteurJoueurs);
                    compteurJoueurs++;
                    /* warn all players a new player is connected */
                    broadcast(mess);
                    /* warn new players about already connected players */
                    int i;
                    for(i = 0; i < compteurJoueurs - 1; i++) {
                    	sprintf(mess, "C %s %d", tableauJoueurs[i].nom, i);
                    	sendMessage(compteurJoueurs - 1, mess);
                    }
                    /* broadcast 'waiting for players' message to all connected players */
                    sprintf(mess, "m En_attente_de_joueurs...");
                    broadcast(mess);
                }
                close(newsockfd);

                if(compteurJoueurs == nbj)
                    currentState = SENDING_ROLES;

                break;
            case SENDING_ROLES:
            	sleep(2);
                sendRoles();

                currentState = PROPOSING_TEAM;
                break;
            case PROPOSING_TEAM:
                sleep(2);
                sendMeneur();

                sprintf(mess, "M %d %d", compteurMissions, participantsMissions[compteurMissions]);
                broadcast(mess);

                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0)
                    error("ERROR on accept");
                bzero(serverbuffer, 256);
                n = read(newsockfd, serverbuffer, 255);
                if (n < 0) error("ERROR reading from socket");
                printf("Here is a message from a client: '%s' '%c'\n", serverbuffer, serverbuffer[0]);

                if (serverbuffer[0] == 'P') {
                    bzero(mess, 100);
                    int nb_participants = participantsMissions[compteurMissions];
                   	int i;

                    if(nb_participants == 2) {
                    	int j1, j2;
	                    sscanf(serverbuffer, "P %d %d", &j1, &j2);
	                    for (i = 0; i < 5; ++i) {
	                    	if (i != j1 && i != j2)
	                    		tableauJoueurs[i].equipe = 0;
	                    	else
	                    		tableauJoueurs[i].equipe = 1;
	                    }
	                } else {
	                	int j1,j2,j3;
	                    sscanf(serverbuffer, "P %d %d %d", &j1, &j2,&j3);
	                    for (i = 0; i < 5; ++i) {
	                    	if (i != j1 && i != j2 && i != j3)
	                    		tableauJoueurs[i].equipe = 0;
	                    	else
	                    		tableauJoueurs[i].equipe = 1;
	                    }
	                }
                }
                close(newsockfd);

                bzero(mess, 100);
                int i;
                for(i = 0; i < compteurJoueurs; i++) {
                	sprintf(mess, "%d %d", tableauJoueurs[i].equipe, i);
                	broadcast(mess);
                }

            	currentState = VOTING_TEAM;
                break;
            case VOTING_TEAM:
                meneurCourant = (meneurCourant + 1);
                if(meneurCourant > nbj) {
                    compteurDefaites = END;
                    currentState = ENDING_GAME;
                    sprintf(mess, "m L'équipe_d'Intervention_n'est_pas_arrivée_à_temps...");
                    broadcast(mess);
                    sleep(2);
                    break;
                }
            	compteurVotes = 0;
            	compteurReussites = 0;

            	while(compteurVotes < compteurJoueurs) {
	                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	                if (newsockfd < 0)
	                    error("ERROR on accept");
	                bzero(serverbuffer, 256);
	                n = read(newsockfd, serverbuffer, 255);
	                if (n < 0) error("ERROR reading from socket");
	                printf("Here is a message from a client: '%s' '%c'\n", serverbuffer, serverbuffer[0]);

	                if (serverbuffer[0] == 'V') {
                        char nom[100];
	                    bzero(mess, 100);
	                    sscanf(serverbuffer, "V %s %s", mess, nom);

                        int voting_player_id = findPlayerId(nom);
	                    if(strcmp(mess, "oui") == 0) {
	                    	compteurReussites++;
                            sprintf(mess, "V o %d", voting_player_id);
                            broadcast(mess);
                        } else {
                            sprintf(mess, "V n %d", voting_player_id);
                            broadcast(mess);
                        }
	                    compteurVotes++;
	                }
	                close(newsockfd);
	            }

	            sleep(1);

	            if(compteurReussites > compteurJoueurs / 2) {
                    sprintf(mess, "m Equipe_acceptée_!");
                    broadcast(mess);
                    bzero(mess, 100);
                    int i;
                    for(i = 0; i < compteurJoueurs; i++) {
                		sprintf(mess, "L %d %d", tableauJoueurs[i].role, tableauJoueurs[i].equipe);
                		sendMessage(i, mess);
                    }
                    currentState = COMPLETING_MISSION;
                    sameVote = 0;
                    meneurCourant = meneurCourant % nbj;
	            } else {
                    sprintf(mess, "m Equipe_refusée.");
                    broadcast(mess);
                    currentState = PROPOSING_TEAM;
                    sameVote = 1;
	            }

                break;
            case COMPLETING_MISSION:
            	sleep(2);

            	compteurVotes = 0;
            	compteurEchecs = 0;

            	int nb_participants = participantsMissions[compteurMissions];
            	while(compteurVotes < nb_participants) {
	                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	                if (newsockfd < 0)
	                    error("ERROR on accept");
	                bzero(serverbuffer, 256);
	                n = read(newsockfd, serverbuffer, 255);
	                if (n < 0) error("ERROR reading from socket");
	                printf("Here is a message from a client: '%s' '%c'\n", serverbuffer, serverbuffer[0]);

	                if (serverbuffer[0] == 'V') {
	                    bzero(mess, 100);
	                    sscanf(serverbuffer, "V %s", mess);

	                    if(strcmp(mess, "non") == 0)
	                    	compteurEchecs++;
	                    compteurVotes++;
	                }
	                close(newsockfd);
	            }

	            if(compteurEchecs < 1) {
                    sprintf(mess, "m Mission_réussie_!");
                    broadcast(mess);
                    compteurVictoires++;
                    compteurMissions++;
                    switchCompleteMission();
                    sprintf(mess, "N %d %d", compteurVictoires, compteurDefaites);
                    broadcast(mess);
	            } else {
                    sprintf(mess, "m Mission_ratée.");
                    broadcast(mess);
                    compteurDefaites++;
                    compteurMissions++;
                    switchCompleteMission();
                    sprintf(mess, "N %d %d", compteurVictoires, compteurDefaites);
                    broadcast(mess);
	            }

                break;
            case ENDING_GAME:
                sleep(2);
            	if(compteurVictoires == END)
                	sprintf(mess, "R 0");
                else if(compteurDefaites == END)
                	sprintf(mess, "R 1");
                broadcast(mess);
                sleep(3);
                running = 0;
            	break;
            default:
                break;
        }
    }

    close(sockfd);
}

void sendMessage(int j, char *mess)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *playerserver;
    char buffer[256];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
            error("ERROR opening socket");
    playerserver = gethostbyname(tableauJoueurs[j].ipaddress);
    if (playerserver == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)playerserver->h_addr, (char *)&serv_addr.sin_addr.s_addr,
                            playerserver->h_length);
    serv_addr.sin_port = htons(tableauJoueurs[j].portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                    error("ERROR connecting");

    n = write(sockfd, mess, strlen(mess));
    if (n < 0)
            error("ERROR writing to socket");
    close(sockfd);
}

void broadcast(char *message)
{
    int i;
    printf("broadcast %s\n", message);
    for (i = 0; i < compteurJoueurs; i++)
        sendMessage(i, message);
}

void sendRoles() {
    int i, j;
    printf("sending roles\n");
    for (i = 0; i < compteurJoueurs; i++) {
        char mess[100];
        /* rebel */
        if(!tableauJoueurs[i].role)
            sprintf(mess, "6 %d", i);
        /* spy */
        else {
        	strcpy(mess, "7");
        	for(j = 0; j < compteurJoueurs; j++)
        		if(tableauJoueurs[j].role)
            		sprintf(mess, "%s %d", mess, j);
        }
        sendMessage(i, mess);
    }
}

void sendMeneur()
{
    char mess[100];
    int i;
    for (i = 0; i < compteurJoueurs; i++) {
        if(i == meneurCourant)
            sprintf(mess, "2 %s", tableauJoueurs[meneurCourant].nom);
        else
        	sprintf(mess, "3 %s", tableauJoueurs[meneurCourant].nom);
        sendMessage(i, mess);
    }
}

void sendEquipe()
{
    char mess[256];
    int i;
    printf("sending equipe\n");
    for (i = 0; i < compteurJoueurs; i++) {
        if(tableauJoueurs[i].equipe)
            sprintf(mess, "%s %s", tableauJoueurs[i].nom, mess);
    }
    broadcast(mess);
}

void sendChosenOnes()
{
    char mess[256];
    int i;
    printf("sending chosen ones\n");
    for (i = 0; i < compteurJoueurs; i++) {
        if(tableauJoueurs[i].equipe)
            sprintf(mess, "%s %s", tableauJoueurs[i].nom, mess);
    }
    broadcast(mess);
}

int main(int argc, char *argv[])
{
    pthread_t thread1, thread2;
    int iret1, iret2;
    srand(time(NULL));

	if (argc!=3)
	{
		printf("Usage : ./mainserver nbjoueurs numport\n");
		exit(1);
	}

    com='0';
    nbj=atoi(argv[1]);
    printf("Nombre de joueurs=%d\n",nbj);
    portno=atoi(argv[2]);
    printf("Serveur ecoute sur port %d\n",portno);
    compteurJoueurs=0;

    compteurRebelles=0;
    compteurEspions=0;
    compteurMissions=0;
    meneurCourant = 0;
    nbespions=2;// 
    initRoles();

    /* Create independent threads each of which will execute function */

    iret1 = pthread_create(&thread1, NULL, server, NULL);
    if(iret1)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n", iret1);
        exit(EXIT_FAILURE);
    }

    printf("pthread_create() for thread 1 returns: %d\n", iret1);

    /* Wait till threads are complete before main continues. Unless we  */
    /* wait we run the risk of executing an exit which will terminate   */
    /* the process and all threads before the threads have completed.   */

    pthread_join(thread1, NULL);

    exit(0);
}
