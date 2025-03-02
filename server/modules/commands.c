#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../defines.h"
#include "commands.h"
#include "colors.h"
#include "printutil.h"
#include "serverutil.h"

#define FOREACH(x) for(int i = 0; i < (x); i++)

#define STRINGIFY(x) #x
#define TOSTR(x) STRINGIFY(x)

extern struct Room rooms[MAX_ROOMS];
extern struct Room lobby;

void sendLobbyMenu(int socketFD){
    char buffer[BUFFERSIZE];
    buffer[0] = '\0';
    char line[96];

    int added = 0;
    formatColor(PURPLE, "\nAvailable rooms:\n", buffer, BUFFERSIZE);
    FOREACH(MAX_ROOMS){
        if ( rooms[i].users == NULL )
            continue;
        
        snprintf(line, 96, "%d- %s %d/%d\n",
        added+1, rooms[i].name, rooms[i].count, rooms[i].max_size);

        formatColor(GREEN, line, buffer, BUFFERSIZE);
        added++;
    }

    if (added == 0){
        formatColor(YELLOW, "No rooms available at the moment.\n", buffer, BUFFERSIZE);
    }

    formatColor(YELLOW, 
            "\n"
            "/join [room name]\n"
            "/create [room name] [max size]\n"
            "/nickname [nickname]\n"
            "/refresh\n"
            "/exit\n"
            "\n",
            buffer,
            BUFFERSIZE
            );

    formatColor(PURPLE, "Your command: ", buffer, BUFFERSIZE);

    sendUntilEnd(socketFD, buffer);
}

void sendRoomMenu(int socketFD){
    char buffer[BUFFERSIZE];
    buffer[0] = '\0';

    formatColor(PURPLE, "\nRoom commands:\n", buffer, BUFFERSIZE);
    formatColor(YELLOW, 
                "/lobby\n"
                "/exit\n", 
                buffer, BUFFERSIZE
               );

    sendUntilEnd(socketFD, buffer);
}

enum commandReturn executeCommandLobby(char * command, struct User * user){
    char * arg;
    int ret;

    printOUT("[COMMAND] Executing command: ");
    printLineOUT(command);

    arg = strtok(command, " ");
    
    if ( arg == NULL )
        return ERROR_COMMAND_NOT_RECOGNIZED;

    if ( !strcmp(arg, "/join") ){
        arg = strtok(NULL, " ");

        if ( (arg == NULL) || (strlen(arg) == 0) ){
            return ERROR_JOIN_ROOM_NOT_GIVEN;
        }

        if ( strlen(user->name) == 0 ){
            return ERROR_JOIN_USERNAME_NOT_SET;
        }
        
        FOREACH(MAX_ROOMS){
            if ( (rooms[i].users != NULL) && (!strcmp(rooms[i].name, arg)) ){
                if ( (ret = moveUser(user, &rooms[i])) < 0 ){
                    if ( ret == ROOM_FULL )
                        return ERROR_JOIN_ROOM_FULL;
                    else
                        return ERROR_JOIN;
                }
                return SUCCESS_JOIN;
            }
            else if ( i == MAX_ROOMS -1 ){
                return ERROR_JOIN_ROOM_NOT_FOUND;
            }
        }
    }
    
    if ( !strcmp(arg, "/create") ){
        arg = strtok(NULL, " ");
        
        if ( strlen(user->name) == 0 ){
            return ERROR_JOIN_USERNAME_NOT_SET;
        }
        
        if ( (arg == NULL) || (strlen(arg) == 0) ){
            return ERROR_CREATE_ROOM_NAME_NOT_GIVEN;
        }

        if ( strlen(arg) > MAX_ROOMNAME ){
            return ERROR_CREATE_ROOM_NAME_MAXIMUM;
        }

        char name[MAX_ROOMNAME + 1];
        strncpy(name, arg, MAX_ROOMNAME);
        name[MAX_ROOMNAME] = '\0';
        if ( !strcmp(name, "_LOBBY") ){
            return ERROR_CREATE_ROOM_NAME_RESERVED;
        }

        FOREACH(MAX_ROOMS){
            if ( (rooms[i].users != NULL) && !strcmp(rooms[i].name, name) ){
                return ERROR_CREATE_ROOM_ALREADY_EXISTS;
            }
        }


        arg = strtok(NULL, " ");

        if ( arg == NULL || strlen(arg) == 0 ){
            return ERROR_CREATE_ROOM_SIZE_NOT_GIVEN;
        }

        int room_size = atoi(arg);
        if ( (room_size <= 1) || (room_size > MAX_USER_PER_ROOM) ){
            return ERROR_CREATE_ROOM_SIZE_INVALID;
        }

        int room_index = -1;
        FOREACH(MAX_ROOMS){
            if ( rooms[i].users == NULL ){
                room_index = i;
                initRoom(name, room_size, &rooms[i]);
                break;
            }
        }

        if ( room_index == -1 ){
            return ERROR_CREATE_SERVER_FULL;
        }

        moveUser(user, &rooms[room_index]);
        return SUCCESS_CREATE;
    }
    
    if ( !strcmp(arg, "/nickname") ){
        arg = strtok(NULL, " ");

        if ( arg == NULL ){
            return ERROR_NICKNAME_NOT_GIVEN;
        }

        if ( strlen(arg) > MAX_USERNAME ){
            return ERROR_NICKNAME_MAXIMUM;
        }

        strncpy(user->name, arg, MAX_USERNAME);
        user->name[MAX_USERNAME] = '\0';

        return SUCCESS_NICKNAME;
    }

    if ( !strcmp(arg, "/refresh" )){
        sendLobbyMenu(user->socketFD);
        return SUCCESS_REFRESH;
    }

    if ( !strcmp(arg, "/exit") ){
        removeAndDisconnectUser(user);
        return SUCCESS_EXIT;
    }

    return ERROR_COMMAND_NOT_RECOGNIZED;
    
}

enum commandReturn executeCommandRoom(char * command, struct User * user){
    char * arg;
    arg = strtok(command, " ");

    if ( !strcmp(arg, "/lobby") ){
        if ( moveUser(user, &lobby) < 0 )
            printAndExitFailure("moveUser failed.");
        return SUCCESS_LOBBY;
    }

    if ( !strcmp(arg, "/exit") ){
        removeAndDisconnectUser(user);
        return SUCCESS_EXIT;
    }

    return ERROR_COMMAND_NOT_RECOGNIZED;
}

void handleCommandReturn(enum commandReturn return_val, int socketFD){
    switch (return_val){
        case ERROR_JOIN_ROOM_NOT_GIVEN:
            sendError(socketFD, "The room name must be given in the command.");
            break;
        case ERROR_JOIN_USERNAME_NOT_SET:
            sendError(socketFD, "You must set a nickname first.");
            break;
        case ERROR_JOIN_ROOM_NOT_FOUND:
            sendError(socketFD, "Room not found.");
            break;
        case ERROR_JOIN_ROOM_FULL:
            sendError(socketFD, "Room is full.");
            break;
        case ERROR_JOIN:
            sendError(socketFD, "Couldn't join room.");
            break;
        case ERROR_CREATE_ROOM_ALREADY_EXISTS:
            sendError(socketFD, "There is already a room with this name.");
            break;
        case ERROR_CREATE_ROOM_NAME_MAXIMUM:
            sendError(socketFD, "The room name must be maximum " 
                                      TOSTR(MAX_ROOMNAME) 
                                      " characters long.");
            break;
        case ERROR_CREATE_ROOM_NAME_NOT_GIVEN:
            sendError(socketFD, "Room name must be given in the command.");
            break;
        case ERROR_CREATE_ROOM_NAME_RESERVED:
            sendError(socketFD, "This room name cannot be taken.");
            break;
        case ERROR_CREATE_ROOM_SIZE_NOT_GIVEN:
            sendError(socketFD, "Room size must be given in the command.");
            break;
        case ERROR_CREATE_ROOM_SIZE_INVALID:
            sendError(socketFD, "Invalid room size, minimum is 1, maximum is " 
                                       TOSTR(MAX_USER_PER_ROOM));
            break;
        case ERROR_CREATE_SERVER_FULL:
            sendError(socketFD, "Server is full.");
            break;
        case ERROR_NICKNAME_NOT_GIVEN:
            sendError(socketFD, "Nickname must be given in the command.");
            break;
        case ERROR_NICKNAME_MAXIMUM:
            sendError(socketFD, "Nickname too long, maximum length is "
                                 TOSTR(MAX_USERNAME));
            break;
        case ERROR_COMMAND_NOT_RECOGNIZED:
            sendError(socketFD, "Command not recognized.");
            break;
        case SUCCESS_CREATE:
            sendMessage(socketFD, "Room set up successfully.");
            sendMessage(socketFD, "Moved to room.");
            sendRoomMenu(socketFD);
            break;
        case SUCCESS_JOIN:
            sendMessage(socketFD, "Joined room.");
            sendRoomMenu(socketFD);
            break;
        case SUCCESS_REFRESH:
            break;
        case SUCCESS_LOBBY:
            sendMessage(socketFD, "Redirected to lobby.");
            sendLobbyMenu(socketFD);
            break;
        case SUCCESS_NICKNAME:
            sendMessage(socketFD, "Nickname set.");
        case SUCCESS_EXIT:
            break;
        default:
            fprintf(stderr, "Error when handling command return on line " 
                            TOSTR(__LINE__)
                            " in file " __FILE__
                            " -> return_val is %d", return_val);
    }
}
