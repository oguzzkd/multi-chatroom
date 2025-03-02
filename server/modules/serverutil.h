#define OUTSTREAM stdout

#ifndef USERTYPE
#define USERTYPE
#include "usertype.h"
#endif

struct sockaddr_in createAddr(sa_family_t addr_family, uint16_t port, in_addr_t ip);
void bindAndListen(int serverFD, struct sockaddr_in *serverAddr, int queueSize);
void acceptIncomingConnection(int serverFD, struct Room *room);
void recvAndPrintIncoming(struct User * user);
void * recvAndPrintIncoming_THREAD(void *arg);

void sendUntilEnd(int socketFD, char * msg);
void sendAll(char * buffer, struct User * user);
void sendMessage(int socketFD, char * msg);
void sendError(int socketFD, char * msg);
void sendUserJoined(struct User * user);
void sendUserLeft(struct User * user);

void printAndExitFailure(char * msg);
