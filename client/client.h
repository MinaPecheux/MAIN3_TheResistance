/* Client manager. Uses TCP connection.
    The server iddr and port number are passed as arguments.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Header file.
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void* runClient(MainArgs* args);

#endif
