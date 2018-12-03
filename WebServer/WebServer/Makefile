CFLAGS = -g -Wall -D _BSD_
LIB = -pthread -lmagic

sws: config.o connection.o main.o request.o response.o server.o sws.o threadpool.o threadsyn.o util.o
	$(CC) -o sws config.o connection.o main.o request.o response.o server.o sws.o threadpool.o threadsyn.o util.o $(LIB)

config.o: config.c
	$(CC) $(CFLAGS) -c -o config.o config.c
util.o: util.c
	$(CC) $(CFLAGS) -c -o util.o util.c
connection.o: connection.c
	$(CC) $(CFLAGS) -c -o connection.o connection.c
main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c
request.o: request.c
	$(CC) $(CFLAGS) -c -o request.o request.c
response.o: response.c
	$(CC) $(CFLAGS) -c -o response.o response.c
server.o: server.c
	$(CC) $(CFLAGS) -c -o server.o server.c
sws.o: sws.c
	$(CC) $(CFLAGS) -c -o sws.o sws.c
threadpool.o: threadpool.c
	$(CC) $(CFLAGS) -c -o threadpool.o threadpool.c
threadsyn.o: threadsyn.c
	$(CC) $(CFLAGS) -c -o threadsyn.o threadsyn.c

.PHONY: clean
clean:
	-rm -fr sws *.o magic.mgc
