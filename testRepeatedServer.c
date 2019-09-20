#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "socket.h"

#define PORT_NUM 52000
#define NUM_ITERATIONS  1000

static int hSocketIntf;

extern SocketInterface_T socketIntf;

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
    int i;
    int num_send;
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
        if ( -1 == hClientIntf ) {
            continue;
        }

        if ( (pid = fork()) < 0 ) {
            socketIntf.Close(hClientIntf);
            continue;
        } else if ( pid > 0 ) {
            socketIntf.Close(hClientIntf);
            continue;
        }

       for ( i = 0; i < NUM_ITERATIONS; i++ ) {
            snprintf((char*)buffer, 256, "%d\n", i);
            num_send = socketIntf.Write(hClientIntf, (uint8_t*)buffer, strlen((char*)buffer));
            printf("-->%d: %s", num_send, (char*)buffer);
       }

        socketIntf.Close(hClientIntf); // close connection to client

        // Open port and wait for client to connect
        printf("Attempting to connect on port %d...\n", PORT_NUM);
    }

    printf("Exiting\n\n");
    return 0;
}
