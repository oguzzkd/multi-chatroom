#ifndef USERTYPE
#include "usertype.h"
#endif

enum commandReturn {
    // LOBBY COMMANDS
    SUCCESS_JOIN,
    SUCCESS_CREATE,
    SUCCESS_NICKNAME,
    SUCCESS_REFRESH,
    ERROR_JOIN_ROOM_NOT_GIVEN,
    ERROR_JOIN_USERNAME_NOT_SET,
    ERROR_JOIN_ROOM_NOT_FOUND,
    ERROR_JOIN_ROOM_FULL,
    ERROR_JOIN,
    ERROR_NICKNAME_MAXIMUM,
    ERROR_NICKNAME_NOT_GIVEN,
    ERROR_CREATE_ROOM_ALREADY_EXISTS,
    ERROR_CREATE_ROOM_NAME_MAXIMUM,
    ERROR_CREATE_ROOM_NAME_NOT_GIVEN,
    ERROR_CREATE_ROOM_NAME_RESERVED,
    ERROR_CREATE_ROOM_SIZE_NOT_GIVEN,
    ERROR_CREATE_ROOM_SIZE_INVALID,
    ERROR_CREATE_SERVER_FULL,
    // ROOM COMMANDS
    SUCCESS_LOBBY,
    // COMMON COMMANDS
    SUCCESS_EXIT,
    ERROR_COMMAND_NOT_RECOGNIZED
};

enum commandReturn executeCommandLobby(char * command, struct User * user);
enum commandReturn executeCommandRoom(char * command, struct User * user);
void handleCommandReturn(enum commandReturn return_val, int socketFD);
