#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "socket.h"

#define PORT_NUM 52000

extern SocketInterface_T socketIntf;

static int hSocketIntf;

void signal_handler(int sig) {

    switch(sig) {
        case SIGINT:
        case SIGQUIT:
        case SIGHUP:
        case SIGTERM:
            socketIntf.Close(hSocketIntf); // close connection to client
            exit(1);
            break;
        default:
            break;
    }
}

int main ( void )
{
    uint8_t buffer[256];
    uint32_t bytesReceived;
    int pid;
    int hClientIntf;

    // Set up signal handler to get out of this
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);


    // Open port and wait for client to connect
    printf("Attempting to connect on port %d...\n", PORT_NUM);
    hSocketIntf = socketIntf.OpenServerSocket(PORT_NUM);
    if ( -1 == hSocketIntf ) {
        printf("Failed to open port!\n");
        return 0;
    }

    // We are expecting a message from the newly connected client
    while ( 1 ) {
        hClientIntf = socketIntf.AcceptClient(hSocketIntf);
        if ( -1 == hClientIntf ) { // only proceed if something valid was returned
            continue;
        }

        if ( (pid = fork()) < 0 ) {
            socketIntf.Close(hClientIntf);
            continue;
        } else if ( pid > 0 ) {
            socketIntf.Close(hClientIntf);
            continue;
        }
        // else -> pid == 0 --> we've forked and have a client on the line - proceed

        printf("Connected!\n");

        printf("INFO:testSocket: calling read...\n");
        bytesReceived = socketIntf.Read(hClientIntf, buffer, 256);
        if ( bytesReceived > 0 ) {
            printf("We received %d bytes\n", bytesReceived);
            printf("Message: %s\n", buffer);
            printf("INFO:testSocket: calling write\n");
            socketIntf.Write(hClientIntf, (uint8_t*)"Server Response Message...", strlen("Server Response Message..."));
            printf("INFO:testSocket: returned from write...\n");
        } else {
            printf("WARN:testSocket: read returned: %d\n", bytesReceived);
            socketIntf.Close(hClientIntf); // close connection to client
            // Open port and wait for client to connect
            printf("Attempting to connect on port %d...\n", PORT_NUM);
        }
    }


    printf("Exiting\n\n");
    return 0;
}
