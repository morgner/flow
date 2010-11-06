
CC = g++

SERVER_LIBS = -lflowsocket              -lc -lstdc++ -lpthread
CLIENT_LIBS = -lflowsocket -lflowsystem -lc -lstdc++

SOCKET_PATH = socket
SERVER_PATH = server
CLIENT_PATH = client
SYSTEM_PATH = system

SOCKET_INC = -I$(SOCKET_PATH)/include
SERVER_INC = -I$(SERVER_PATH)/include -I$(SOCKET_PATH)/include
CLIENT_INC = -I$(CLIENT_PATH)/include -I$(SOCKET_PATH)/include
SYSTEM_INC = -I$(SYSTEM_PATH)/include -I$(SOCKET_PATH)/include

CFLAGS = -c -O3 -Wall -pedantic

LDFLAGS  = -L.

SOCKET_SRC=$(SOCKET_PATH)/src/socket.cpp $(SOCKET_PATH)/src/socketclient.cpp $(SOCKET_PATH)/src/socketserver.cpp $(SOCKET_PATH)/src/socketexception.cpp
SOCKET_OBJ=$(SOCKET_SRC:.cpp=.o)
SOCKET=libflowsocket.a

SYSTEM_SRC=$(SYSTEM_PATH)/src/environment.cpp
SYSTEM_OBJ=$(SYSTEM_SRC:.cpp=.o)
SYSTEM=libflowsystem.a

SERVER_SRC=$(SERVER_PATH)/src/container.cpp $(SERVER_PATH)/src/partner.cpp $(SERVER_PATH)/main.cpp
SERVER_OBJ=$(SERVER_SRC:.cpp=.o)
SERVER=flowserver

CLIENT_SRC=$(CLIENT_PATH)/src/home.cpp $(CLIENT_PATH)/src/pulex.cpp $(CLIENT_PATH)/src/domain.cpp $(CLIENT_PATH)/main.cpp
CLIENT_OBJ=$(CLIENT_SRC:.cpp=.o)
CLIENT=flowclient

all: $(SOCKET) $(SYSTEM) $(SERVER) $(CLIENT)

socket: $(SOCKET)

$(SOCKET): $(SOCKET_OBJ)
	ar rcs $(SOCKET) $(SOCKET_OBJ)

$(SYSTEM): $(SYSTEM_OBJ)
	ar rcs $(SYSTEM) $(SYSTEM_OBJ)

$(SERVER): $(SERVER_OBJ) $(SOCKET)
	$(CC) $(SERVER_OBJ) $(LDFLAGS) $(SERVER_LIBS) -o $@

$(CLIENT): $(CLIENT_OBJ) $(SOCKET) $(SYSTEM)
	$(CC) $(CLIENT_OBJ) $(LDFLAGS) $(CLIENT_LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ $(SOCKET_INC) $(SYSTEM_INC) $(SERVER_INC) $(CLIENT_INC)

clean: 
	rm -f $(SOCKET_OBJ) $(SYSTEM_OBJ) $(SERVER_OBJ) $(CLIENT_OBJ) $(SYSTEM) $(SOCKET) $(SERVER) $(CLIENT)

