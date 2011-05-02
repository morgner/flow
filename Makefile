
CC = clang

SERVER_LIBS = -lflowsocket -lflowcontainer -lc -lstdc++ -lpthread
CLIENT_LIBS = -lflowsocket -lflowcontainer -lc -lstdc++          

SOCKET_PATH = socket
SERVER_PATH = server
CLIENT_PATH = client
SYSTEM_PATH = system
CNTAIN_PATH = container

SOCKET_INC = -I$(SOCKET_PATH)/include
SERVER_INC = -I$(SERVER_PATH)/include -I$(SOCKET_PATH)/include
CLIENT_INC = -I$(CLIENT_PATH)/include -I$(SOCKET_PATH)/include
SYSTEM_INC = -I$(SYSTEM_PATH)/include -I$(SOCKET_PATH)/include
CNTAIN_INC = -I$(CNTAIN_PATH)/include -I$(SOCKET_PATH)/include

CFLAGS = -c -mtune=native -Wall -pedantic

LDFLAGS  = -L.

SOCKET_SRC=$(SOCKET_PATH)/src/socket.cpp $(SOCKET_PATH)/src/socketclient.cpp $(SOCKET_PATH)/src/socketserver.cpp $(SOCKET_PATH)/src/socketexception.cpp
SOCKET_OBJ=$(SOCKET_SRC:.cpp=.o)
SOCKET=libflowsocket.a

CNTAIN_SRC=$(CNTAIN_PATH)/src/container.cpp
CNTAIN_OBJ=$(CNTAIN_SRC:.cpp=.o)
CNTAIN=libflowcontainer.a

SERVER_SRC=$(SERVER_PATH)/src/partner.cpp $(SERVER_PATH)/main.cpp
SERVER_OBJ=$(SERVER_SRC:.cpp=.o)
SERVER=flowserver

CLIENT_SRC=$(CLIENT_PATH)/src/pulex.cpp $(CLIENT_PATH)/src/domain.cpp $(CLIENT_PATH)/src/cryptoexception.cpp $(CLIENT_PATH)/main.cpp
CLIENT_OBJ=$(CLIENT_SRC:.cpp=.o)
CLIENT=flowclient

all: $(SOCKET) $(CNTAIN) $(SERVER) $(CLIENT)

socket: $(SOCKET)
container: $(SOCKET)
client: $(SOCKET)
server: $(SOCKET)

$(SOCKET): $(SOCKET_OBJ)
	ar rcs $(SOCKET) $(SOCKET_OBJ)

$(SYSTEM): $(SYSTEM_OBJ)
	ar rcs $(SYSTEM) $(SYSTEM_OBJ)

$(CNTAIN): $(CNTAIN_OBJ)
	ar rcs $(CNTAIN) $(CNTAIN_OBJ)

$(SERVER): $(SERVER_OBJ) $(SOCKET)
	$(CC) $(SERVER_OBJ) $(LDFLAGS) $(SERVER_LIBS) -o $@

$(CLIENT): $(CLIENT_OBJ) $(SOCKET) $(SYSTEM)
	$(CC) $(CLIENT_OBJ) $(LDFLAGS) $(CLIENT_LIBS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@ $(SOCKET_INC) $(SYSTEM_INC) $(CNTAIN_INC) $(SERVER_INC) $(CLIENT_INC)

clean: 
	rm -f $(SOCKET_OBJ) $(SYSTEM_OBJ) $(SERVER_OBJ) $(CLIENT_OBJ) $(SOCKET) $(SERVER) $(CLIENT)
