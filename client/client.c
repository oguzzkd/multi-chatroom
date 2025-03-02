#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "clientutil.h"

int recvThreadSetUp = 0;

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

    struct sockaddr_in addr;
    addr = createAddr(AF_INET, port, INADDR_ANY);

    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if ( socketFD == -1 ){
        printAndExitFailure("Socket descriptor could not be created.");
    }

    connectToServer(socketFD, &addr);

    recvAndPrintIncomingOnThread(socketFD);
    while ( !recvThreadSetUp ) 
        continue;

    setUsername(socketFD);
    
    puts("Write your message and press enter to send.\n");
    getLobbyMenu(socketFD);

    readInputAndSend(socketFD);

    return 0;
}