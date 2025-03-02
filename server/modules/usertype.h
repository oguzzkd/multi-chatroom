#define USERTYPE

#ifndef NETINET_INCLUDED
#include <netinet/in.h>
#define NETINET_INCLUDED
#endif

#include "../../defines.h"

struct User;
struct Room;

#define NULL_SOCKET -2
#define NULL_ROOM NULL

#define ROOM_FULL -1

#define USER_HAS_NO_ROOM -1
#define USER_NOT_FOUND_IN_ROOM -2

struct Room {
    struct User ** users;
    char name[MAX_ROOMNAME + 1];
    int max_size;
    int count;
};

struct User {
    struct sockaddr_in addr;
    int socketFD;

    struct Room * room;
    char name[MAX_USERNAME + 1];
};


int removeUserFromRoom(struct User * user);
int removeAndDisconnectUser(struct User * user);
int addUser(struct User * user, struct Room *room);
int moveUser(struct User * user, struct Room *dest_room);
int initRoom(char * name, int max_size, struct Room *room);
int resetRoom(struct Room *room);