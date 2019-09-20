/*
*    blah blah blah.  Sockets are great
*
*    History:
*        November 16, 2010:  created
*/

#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

// Error debugging
#include <errno.h>

#define MSG_MAGIC               0x42
#define MAX_READ_BUFFER_LEN     10240  // 10KB
#define MSG_HEADER_OVERHEAD     6 // (1)magic, (4)length, (1) checksum
// Container for socket messages
typedef struct _msg_header_t {
    uint8_t magic;
    uint32_t length;
    uint8_t checksum;
    uint8_t data; // start of data goes here
} __attribute__((__packed__)) msg_header_t;

/* OpenServerSocket
*    input params:
*        uint16_t portNum - port number to connect to
*
*    return int
*        -1 : if error occurred opening socket
*        file descriptor to opened socket
*/
typedef int (*OpenServerSocket_T)(uint16_t port);

/* AcceptClient
*    input params:
*        int hSocket - file descriptor to opened socket
*
*    return int
*        -1 : if error occurred opening socket
*        file descriptor to connected client
*/
typedef int (*AcceptClient_T)(int hSocket);

/* ConnectToServer
*    input params:
*        char* szIPAddress - ASCII string representation of the target IP address Ex: "127.0.0.1"
*        uint16_t portNum - port number to connect to
*
*    return int
*        -1 : if error occurred opening socket
*        file descriptor to opened socket
*/
typedef int (*ConnectToServer_T)(const char* szIPAddress, uint16_t portNum);

/* Read()
*    input params:
*        int hSocket - file descriptor to opened socket
*        uint8_t *buffer - buffer to put returned data into
*        uint32_t bufferLen - length of allocated space in *buffer
*
*    returns:  number of bytes placed into *buffer, -1 if error
*/
typedef int32_t (*Read_T)( int hSocket, uint8_t* buffer, uint32_t bufferLen );

/* Write()
*    input params:
*        int hSocket - file descriptor to opened socket
*        uint8_t *buffer - buffer containing data to be sent
*        uint32_t bufferLen - length of bytes to send
*
*    returns:  number of bytes sent, -1 if error
*/
typedef int32_t (*Write_T)( int hSocket, uint8_t* data, uint32_t len );

/* Close ()
*    Closes connection to a specific client and/or the socket interface
*    If program is executing as a server, first close the connection to the client
*      then call to close the server's socket connection
*    input params:
*        int hSocket - file descriptor to the connected client / socket interface
*/
typedef void (*Close_T)( int hSocket );

typedef struct _SocketInterface_T {
    OpenServerSocket_T OpenServerSocket;
    AcceptClient_T AcceptClient;
    ConnectToServer_T ConnectToServer;
    Read_T Read;
    Write_T Write;
    Close_T Close;
} __attribute__((__packed__))SocketInterface_T;

#endif // SOCKET_H

