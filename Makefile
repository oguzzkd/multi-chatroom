common = defines.h

serversources = server/server.c \
				server/modules/serverutil.c \
				server/modules/usertype.c \
				server/modules/commands.c \
				server/modules/printutil.c \
				server/modules/colors.c

serverheaders = server/modules/serverutil.h \
				server/modules/usertype.h \
				server/modules/commands.h \
				server/modules/printutil.h \
				server/modules/colors.h

clientsources = client/client.c \
				client/clientutil.c

clientheaders = client/clientutil.h

all: server_app client_app

server_app: $(serversources) $(serverheaders) $(common)
	gcc $(serversources) -o server_app -g -fsanitize=address,undefined -Wall 
	
client_app: $(clientsources) $(clientheaders) $(common)
	gcc $(clientsources) -o client_app -g -fsanitize=address,undefined -Wall 

clean: 
	rm -f server_app client_app

.PHONY: clean
