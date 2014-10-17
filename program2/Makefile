CPPFLAGS := -Wall

.PHONY: all
all: file_client file_server

file_client:
	$(CC) $(CPPFLAGS) -o $@ stream-talk-client.c

file_server:
	$(CC) $(CPPFLAGS) -o $@ stream-talk-server.c
