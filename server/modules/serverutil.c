#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#ifndef NETINET_INCLUDED
#include <netinet/in.h>
#define NETINET_INCLUDED
#endif

#ifndef USERTYPE
#include "usertype.h"
#endif

#include "../../defines.h"
#include "serverutil.h"
#include "printutil.h"
#include "commands.h"
#include "colors.h"

#define FOREACH(x) for(int i = 0; i < (x); i++)

extern struct Room rooms[MAX_ROOMS];
extern struct Room lobby;

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

void bindAndListen(int serverFD, struct sockaddr_in *serverAddr, int queueSize){
    int bindResult = bind(serverFD, (struct sockaddr *) serverAddr, sizeof(*serverAddr));
    if ( bindResult != 0 ){
        printAndExitFailure("Server could not be bound.");
    }
    
    int listenResult = listen(serverFD, queueSize);
    if ( listenResult != 0 ){
        printAndExitFailure("Server could not listen.");
    }

    printNotice("Server bound and listening.");
    fprintf(OUTSTREAM, "IP: %s PORT: %d\n", inet_ntoa(serverAddr->sin_addr), ntohs(serverAddr->sin_port));
}

void sendUntilEnd(int socketFD, char * msg){
    char buffer[BUFFERSIZE];

    FOREACH((int) (strlen(msg) / BUFFERSIZE) + 1){
        strncpy(buffer, msg, BUFFERSIZE);
        if ( socketFD != NULL_SOCKET )
            send(socketFD, buffer, strlen(buffer), 0);
    }
}

void sendMessage(int socketFD, char * msg){
    char buffer[BUFFERSIZE];
    buffer[0] = '\0';
    
    formatColor(YELLOW, "[SERVER] ", buffer, BUFFERSIZE);
    formatColor(YELLOW, msg, buffer, BUFFERSIZE);
    buffer[BUFFERSIZE - 1] = '\0';
    
    if ( socketFD != NULL_SOCKET )
        send(socketFD, buffer, strlen(buffer), 0);
}

void sendError(int socketFD, char * msg){
    char buffer[BUFFERSIZE];
    buffer[0] = '\0';

    formatColor(RED, "ERROR: ", buffer, BUFFERSIZE);
    formatColor(RED, msg, buffer, BUFFERSIZE);
    buffer[BUFFERSIZE - 1] = '\0';  

    sendMessage(socketFD, buffer);
}

void acceptIncomingConnection(int serverFD, struct Room *room){
    // accept incoming connection
    struct sockaddr_in  clientAddr;
    socklen_t clientAddrSize = sizeof(struct sockaddr_in);
    int clientFD = accept(serverFD, (struct sockaddr *) &clientAddr, &clientAddrSize);

    if ( clientFD <= 0 ) // don't establish connection in case of error
        printFailure("Error accepting connection.");

    //create and add user for connection
    struct User * user = malloc(sizeof(struct User));
    user->addr = clientAddr;
    user->socketFD = clientFD;
    user->name[0] = '\0';
    user->room = NULL_ROOM;

    int ret;
    if ( (ret = addUser(user, room)) < 0 ){
        if ( ret == ROOM_FULL ){
            printWarning("Server is full, user rejected.");
            close(user->socketFD);
            free(user);
            return;
        }
    }

    // create a thread for handling the connection
    pthread_t thread_id;
    if ( pthread_create(&thread_id, NULL, recvAndPrintIncoming_THREAD, user) != 0 ){
        printFailure("Error creating receiving thread.");
        removeAndDisconnectUser(user);
    }
    else{
        printNotice("Listening to user.");
    }
}

void sendChatMsgToAll(char * msg, struct User * user);
// when new user joins a room the previous user's room is set to NULL_ROOM or NULL //HERE
void recvAndPrintIncoming(struct User * user){
    struct Room * room = user->room;
    char buffer[BUFFERSIZE];
    enum commandReturn command_return;
    ssize_t recvSize;

    while ( (( recvSize = recv(user->socketFD, buffer, BUFFERSIZE, 0)) > 0 ) 
            && (user->socketFD != NULL_SOCKET) && (room != NULL_ROOM) ){
        room = user->room;
        buffer[recvSize] = '\0';
        
        if ( strcmp(room->name, "_LOBBY") ){
            if ( buffer[0] == '/' ){
                command_return = executeCommandRoom(buffer, user);
                handleCommandReturn(command_return, user->socketFD);
                printf("ret = %d", command_return);
                if (command_return == SUCCESS_EXIT){
                    recvSize = 0;
                    break;
                }
            }
            else
                sendChatMsgToAll(buffer, user);
        }
        else if ( buffer[0] == '/' ){
            command_return = executeCommandLobby(buffer, user);
            handleCommandReturn(command_return, user->socketFD);
            if (command_return == SUCCESS_EXIT){
                recvSize = 0;
                break;
            }
        }
        else {
            sendError(user->socketFD, "You can only send commands starting with '/' in the lobby.");
        }

    }

    if ( recvSize == 0 ){
        puts("User disconnected.");
    }
    else{
        printf("recv error: %s\n", strerror(errno));
    }
    int ret;
    if ( (ret = removeAndDisconnectUser(user) ) < 0 );
        // printf("Error removing user, error no: %d\n", ret);
}

void * recvAndPrintIncoming_THREAD(void *arg){
    struct User * arg_val = (struct User *) arg;

    recvAndPrintIncoming(arg_val);

    return NULL;
}

void sendChatMsgToAll(char * msg, struct User * user){
    char modMsg[BUFFERSIZE];
    modMsg[0] = '\0';

    formatColor( (user->socketFD % COLOR_COUNT), user->name, modMsg, BUFFERSIZE );
    formatColor( (user->socketFD % COLOR_COUNT), ": ", modMsg, BUFFERSIZE );

    formatColor(LIGHT_BLUE, msg, modMsg, BUFFERSIZE - strlen(modMsg));
    
    modMsg[BUFFERSIZE - 1] = '\0';

    if ( user->room != NULL_ROOM )
        sendAll(modMsg, user);
}

void sendAll(char * buffer, struct User * user){
    struct Room * room = user->room;

    if ( room == NULL_ROOM ){
        printFailure("sendAll user has NULL room.");
        return;
    }

    FOREACH(room->count){
        if ((room->users[i] == NULL) ||
            ( room->users[i]->socketFD == NULL_SOCKET ) || 
            ( room->users[i]->socketFD == user->socketFD ) )
            continue;

        send(room->users[i]->socketFD, buffer, strlen(buffer), 0);
    }
}

void sendUserJoined(struct User * user){
    const int msgSize = 96;
    char msg[msgSize];
    char modMsg[msgSize];
    modMsg[0] = '\0';

    if ( (user->room == NULL_ROOM) || (!strcmp(user->room->name, "_LOBBY")) )
        return;

    snprintf(msg, msgSize, "%s joined the room.", user->name);
    msg[msgSize - 1] = '\0';

    formatColor(GREEN, msg, modMsg, msgSize);

    sendAll(modMsg, user);
}

void sendUserLeft(struct User * user){
    const int msgSize = 96;
    char msg[msgSize];
    char modMsg[msgSize];
    modMsg[0] = '\0';

    if ( (user->room == NULL_ROOM) || (!strcmp(user->room->name, "_LOBBY")) )
        return;

    snprintf(msg, msgSize, "%s left the room.", user->name);
    msg[msgSize - 1] = '\0';

    formatColor(YELLOW, msg, modMsg, msgSize);

    sendAll(modMsg, user);
}