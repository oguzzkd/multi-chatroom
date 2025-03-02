#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef NETINET_DECLARED
#include <netinet/in.h>
#define NETINET_DECLARED
#endif

#include "usertype.h"
#include "serverutil.h"

#define FOREACH(x) for(int i = 0; i < (x); i++)

int initRoom(char * name, int max_size, struct Room *room){
    room->users = (struct User **) calloc(max_size, sizeof(struct User *));
    room->max_size = max_size;
    room->count = 0;
    
    strncpy(room->name, name, MAX_ROOMNAME);
    room->name[MAX_ROOMNAME] = '\0';

    return 1;
}

int resetRoom(struct Room *room){
    FOREACH(room->count){
        if ( (room->users[i] != NULL) && (room->users[i]->socketFD != NULL_SOCKET) ) 
            removeAndDisconnectUser(room->users[i]);
    }
    free(room->users);
    room->users = NULL;
    room->max_size = 0;
    room->count = 0;
    room->name[0] = '\0';

    return 1;
}

int addUser(struct User * user, struct Room *room){
    FOREACH(room->max_size){
        if ( room->users[i] == NULL ){
            room->count++;
            room->users[i] = user;

            user->room = room; 

            sendUserJoined(user);
            return room->count;
        }
    }
    return ROOM_FULL; 
}

int removeUserFromRoom(struct User * user){
    struct Room * room = user->room;

    if ( room == NULL_ROOM )
        return USER_HAS_NO_ROOM;

    FOREACH(room->max_size){
        if ( (room->users[i] != NULL) && room->users[i]->socketFD == user->socketFD){
            room->users[i] = NULL;
            
            if ( ((room->count - 1) == 0) && (strcmp(room->name, "_LOBBY")) ){
                resetRoom(room);
                user->room = NULL_ROOM;
                return 0;
            }
            else{
                sendUserLeft(user);
                room->count--;
                user->room = NULL_ROOM;
                return room->count;
            }
        }
    }
    return USER_NOT_FOUND_IN_ROOM;
}
int removeAndDisconnectUser(struct User * user){
    int ret;
    if( (ret = removeUserFromRoom(user)) < 0 ){
        return ret;
    }
    close(user->socketFD);

    free(user);

    return 1;
}

int moveUser(struct User * user, struct Room *dest_room){
    int ret;
    struct Room * prev_room = user->room;

    if ( (ret = removeUserFromRoom(user)) < 0 )
        return ret;
    
    if ( (ret = addUser(user, dest_room)) < 0 ){
        addUser(user, prev_room);
        return ret;
    }

    return 1;
}
