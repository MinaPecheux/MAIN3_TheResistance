/* Server manager.
[MAIN3 - Operating Systems] B. Merhane and M. Pecheux, Feb. 2017

Header file.
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void* runServer(MainArgs* args);

#endif
