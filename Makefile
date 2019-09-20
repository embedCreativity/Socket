#CC=mips-gcc
Server := testSocket
Client := testSocketClient
#SHARED_LIB := libsocket.so
#LIBVERSION := 1.0.1

# build helloworld executable when user executes "make"
CFLAGS += -Wall -g
#LDFLAGS += -L/usr/lib
#LIBRARY += -lsocket

#testSocket: testSocket.o libSocket
testSocket: testSocket.o socket.o
	#$(CC) $(CFLAGS) $(LDFLAGS) testSocket.o $(LIBRARY) -o testSocket $(LIBS)
	$(CC) $(CFLAGS) testSocket.o socket.o -o testSocket

testSocket.o: testSocket.c
	$(CC) $(CFLAGS) -c testSocket.c

#testSocketClient: testSocketClient.o libSocket
testSocketClient: testSocketClient.o socket.o
	#$(CC) $(CFLAGS) $(LDFLAGS) testSocketClient.o $(LIBRARY) -o testSocketClient $(LIBS)
	$(CC) $(CFLAGS) testSocketClient.o socket.o -o testSocketClient

testRepeatedServer.o: testRepeatedServer.c
	$(CC) $(CFLAGS) -c testRepeatedServer.c

testRepeatedClient.o: testRepeatedClient.c
	$(CC) $(CFLAGS) -c testRepeatedClient.c

testRepeatedServer: testRepeatedServer.o socket.o
	#$(CC) $(CFLAGS) $(LDFLAGS) testRepeatedServer.o $(LIBRARY) -o testRepeatedServer $(LIBS)
	$(CC) $(CFLAGS) testRepeatedServer.o socket.o -o testRepeatedServer

testRepeatedClient: testRepeatedClient.o socket.o
	#$(CC) $(CFLAGS) $(LDFLAGS) testRepeatedClient.o $(LIBRARY) -o testRepeatedClient $(LIBS)
	$(CC) $(CFLAGS) testRepeatedClient.o socket.o -o testRepeatedClient

testSocketClient.o: testSocketClient.c
	$(CC) $(CFLAGS) -c testSocketClient.c

socket.o: socket.c
	#$(CC) -fPIC -g -c -Wall socket.c
	$(CC) $(CFLAGS) -c socket.c

#libSocket: socket.o
	#$(CC) -shared -Wl,-soname,$(SHARED_LIB) -o $(SHARED_LIB).$(LIBVERSION) socket.o -lc
	#sudo mv $(SHARED_LIB).$(LIBVERSION) /usr/lib/
	#sudo cp --preserve=timestamps socket.h /usr/include/
	#sudo chmod 644 /usr/include/socket.h
	#sudo ldconfig -n /usr/lib

all: $(Server) $(Client) testRepeatedServer testRepeatedClient

# remove object files and executable when user executes "make clean"
clean:
	#rm -f *.o testSocketClient testSocket $(SHARED_LIB)*
	rm -f *.o testSocketClient testSocket testRepeatedServer testRepeatedClient


