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

TEST_FILE := program3.pdf

.PHONY: all
all: $(CLIENT) $(SERVER)

run:
	if [ -a $(TEST_FILE)_new ]; then rm $(TEST_FILE)_new; fi;
	./$(SERVER) 10000 & 
	./$(CLIENT) 127.0.0.1 10000 $(TEST_FILE)

$(CLIENT): stream-talk-client.c
	$(CC) $(CPPFLAGS) $? -o $@ $(LDFLAGS) $(LDLIBS)

$(SERVER): stream-talk-server.c
	$(CC) $(CPPFLAGS) $? -o $@ $(LDFLAGS) $(LDLIBS)
