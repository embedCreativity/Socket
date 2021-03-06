/*
  Copyright (C) 2020 Embed Creativity LLC
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

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

