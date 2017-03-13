/* Client manager. Uses TCP connection.
    The server iddr and port number are passed as arguments.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Script file.
*/

#include "client.h"
#include "../common/utils.h"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//void* runClient(MainArgs* args)
int main(int argc, char* argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port or player name\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    sprintf(buffer, "%s", argv[3]);
    n = write(sockfd, buffer, strlen(buffer));

    close(sockfd);
    return 0;
}
