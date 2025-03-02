#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#define NETINET_INCLUDED

#include "../defines.h"
#include "modules/usertype.h"
#include "modules/serverutil.h"

struct Room rooms[MAX_ROOMS];
struct Room lobby; // room selection will be handled here

int main(int argc, char ** argv){
    int port;

    if ( argc != 2 ){
        printf("Usage: %s [PORT]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);

    if ( port <= 0 ){
        printf("Invalid port number %s.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr = createAddr(AF_INET, port, INADDR_ANY);

    // set up server
    int serverFD = socket(AF_INET, SOCK_STREAM, 0);

    if ( serverFD == -1 ){
        printAndExitFailure("Server socket descriptor could not be created.");
    }

    initRoom("_LOBBY", SERVER_CAPACITY, &lobby);

    // init rooms empty
    for (int i = 0; i < MAX_ROOMS; i++){
        rooms[i].users = NULL; 
    }

    bindAndListen(serverFD, &serverAddr, LISTENQUEUE);

    while (1){
        acceptIncomingConnection(serverFD, &lobby);
    }
    
    resetRoom(&lobby);
    for (int i = 0; i < MAX_ROOMS; i++){
        resetRoom(&rooms[i]); 
    }
    
    shutdown(serverFD,SHUT_RDWR);
    return 0;
}