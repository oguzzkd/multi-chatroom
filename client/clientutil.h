#include "../defines.h"

struct sockaddr_in createAddr(sa_family_t addr_family, uint16_t port, in_addr_t ip);

void connectToServer(int socketFD, struct sockaddr_in * addr);
void recvAndPrintIncomingOnThread(int socketFD);
void readInputAndSend(int socketFD);

void getLobbyMenu(int socketFD);
void setUsername(int socketFD);

void printAndExitFailure(char * msg);
