# Overview
Multi Chatroom is a client and server chatting application supporting rooms of different sizes. It was made for the purpose of learning sockets and C programming, as well as having fun.

**Author:** Ismail Oguz Karaduman

# Demo
<img src="/demo/demo.gif" alt="Demo" width="250"/>

# Features
- Multiple rooms discoverable from the server lobby
- Variable room sizes
- Colorful menu messages
- Usernames with different colors

Feel free to contribute if you have a feature in mind :)

# Compiling
Running ```make``` in the project directory will compile both applications.

Running ```make clean``` will remove the compiled applications.

# Running
### Server
The server application has to be started with the following command:
```./server_app [PORT]```

For example: ```./server_app 3000```

Ports **1024 to 49151** are recommended.
### Client
The client can simply be run with:

```./client_app [SERVER'S PORT]```

# Notes
- Messages are not encrypted, don't send sensitive data over the application.
- Configurations can be found in defines.h and can be changed, however extreme values are not recommended (e.g. setting the BUFFERSIZE too low causes menu messages to be sent part by part)
