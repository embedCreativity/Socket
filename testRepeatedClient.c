#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "socket.h"

#define MAX_BUF 100

#define NUM_ITERATIONS  1000

extern SocketInterface_T socketIntf;

int main(int argc, char* argv[])
{
    char buffer[1024];
    int i;
    int number;
    int num_recv;
    int hSocketIntf;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s ip_address port_number\n", argv[0]);
         return 0;
    }

    printf("Beginning test\n");

    hSocketIntf = socketIntf.ConnectToServer(argv[1], atoi(argv[2]));
    if ( -1 != hSocketIntf ) {
        for ( i = 0; i < NUM_ITERATIONS; i++ ) {

            num_recv = socketIntf.Read(hSocketIntf, (uint8_t*)buffer, 1024);
            sscanf(buffer, "%d", &number);
            printf("-->%d: %s", num_recv, (char*)buffer);
            if ( number != i ) {
                printf("ERROR: received incorrect data!: %s\n", buffer);
                //break;
            }
        }
        if ( i == NUM_ITERATIONS ) {
            printf("Test Passed\n");
        } else {
            printf("ERROR: test failed\n");
        }


        printf("Closing socket...\n");

        //printf("<enter> to exit\n");
        //int foo = getchar();
        socketIntf.Close(hSocketIntf);
        printf("socket closed\n");
    } else {
        printf("Failed to connect to server\n");
    }

    return 0;
}

