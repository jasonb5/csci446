# Jason Boutte
# Tyler Parks
#
# CSCI 446
# Fall 2014

CLIENT := snw-client
SERVER := snw-server

CPPFLAGS := -Wall

LDFLAGS := -L.

LDLIBS := -lpacketErrorSend

.PHONY: all
all: $(CLIENT) $(SERVER)

run:
	./$(SERVER) 10000 && ./$(CLIENT) 127.0.0.1 10000 sample  

$(CLIENT): stream-talk-client.c
	$(CC) $(CPPFLAGS) $? -o $@ $(LDFLAGS) $(LDLIBS)

$(SERVER): stream-talk-server.c
	$(CC) $(CPPFLAGS) $? -o $@ $(LDFLAGS) $(LDLIBS)
