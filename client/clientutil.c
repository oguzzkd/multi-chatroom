#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include "../defines.h"
#include "clientutil.h"

extern int recvThreadSetUp;

void printAndExitFailure(char * msg){
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

struct sockaddr_in createAddr(
    sa_family_t addr_family,
    uint16_t port, 
    in_addr_t ip 
    ){
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = addr_family;
    serverAddr.sin_port = htons(port); 
    serverAddr.sin_addr.s_addr = ip; 

    return serverAddr;
}

void connectToServer(int socketFD, struct sockaddr_in * addr){
    int result = connect(socketFD, (struct sockaddr *) addr, sizeof(*addr));
    
    if( result < 0 ){
        printf("Failed to connect to server: %s\n", strerror(errno));
        printAndExitFailure("");
    }
}

void recvAndPrintIncoming(int socketFD){
    char buffer[BUFFERSIZE];
    ssize_t recvSize;

    recvThreadSetUp = 1;
    while ( (recvSize = recv(socketFD, buffer, BUFFERSIZE, 0)) > 0 ){
        buffer[recvSize] = '\0';
        puts(buffer);
    }

    if ( recvSize < 0 ){
        fprintf(stderr, "Error while receiving: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    puts("Server disconnected.\n");
    close(socketFD);
    exit(EXIT_SUCCESS);
}

void * recvAndPrintIncoming_THREAD(void * arg){
    int socketFD = *((int *) arg);
    recvAndPrintIncoming(socketFD);

    return NULL;
}

void recvAndPrintIncomingOnThread(int socketFD){
    pthread_t thread_id;
    
    int res =  pthread_create(&thread_id, NULL, recvAndPrintIncoming_THREAD, (void *) &socketFD);
    if ( res != 0 )
        printAndExitFailure("Error creating receiving thread.");
}

void sendToServer(const char * msg, int socketFD){
    char buffer[BUFFERSIZE];
    strncpy(buffer, msg, BUFFERSIZE);
    buffer[BUFFERSIZE - 1] = '\0';

    if ( send(socketFD, buffer, strlen(buffer), 0) < 0 ){
        printf("send error: %s\n", strerror(errno));
        printAndExitFailure("");
    }
}

int readInput(char * str, int max){
    char * ret_val;
    char * newline;

    ret_val = fgets(str, max, stdin);

    if ( ret_val ){
        newline = strchr(str, '\n');
        if ( newline )
            *newline = '\0';
        else
            while ( getchar() != '\n' )
                continue; // dispose newline
        
        return 1;
    }

    return 0;
}

void readInputAndSend(int socketFD){
    char buffer[BUFFERSIZE];

    while (1){
        if ( !readInput(buffer, BUFFERSIZE) )
            break;
        if ( strlen(buffer) == 0 )
            continue;

        sendToServer(buffer, socketFD);
    }
}

void getLobbyMenu(int socketFD){
    static const char * lobbyMenuCommand = "/refresh";
    
    sendToServer(lobbyMenuCommand, socketFD);
}

int hasWhiteSpace(char * str){
    for (int i = 0; i < strlen(str); i++)
        if ( isspace(str[i]) )
            return 1;
    return 0;
}

void setUsername(int socketFD){
    // 10 + MAX_USERNAME + 1 == length of "/nickname [MAX_USERNAME]" + '\0'
    char command[10 + MAX_USERNAME + 1] = "/nickname "; 
    char username[MAX_USERNAME];

    while (1){
        printf("Your username: ");
        readInput(username, MAX_USERNAME);

        if ( strlen(username) == 0 )
            continue;

        if ( hasWhiteSpace(username) ){
            printf("Spaces are not allowed in the username.\n");
            continue;
        }

        strncat(command, username, MAX_USERNAME);
        sendToServer(command, socketFD);
        break;
    }
}
