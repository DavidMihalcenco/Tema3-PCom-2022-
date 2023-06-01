CC=gcc
CFLAGS=-I.

client: client.c helpers.c buffer.c requests.c CJSON/cJSON.c
	$(CC) -o client client.c helpers.c buffer.c requests.c CJSON/cJSON.c -Wall -g

run: client
	./client

clean:
	rm -f *.o client
