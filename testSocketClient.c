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

extern SocketInterface_T socketIntf;

int main(int argc, char* argv[])
{
    char buffer[1024];
    int hSocketIntf;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s ip_address port_number\n", argv[0]);
         return 0;
    }

    printf("INFO:testSocketClient: calling ConnectToServer()\n");
    hSocketIntf = socketIntf.ConnectToServer(argv[1], atoi(argv[2]));
    if ( -1 != hSocketIntf ) {
        printf("INFO:testSocketClient: connected to server - calling write...\n");
        socketIntf.Write(hSocketIntf, (uint8_t*)"Message sent from TP-Link!", strlen("Message sent from TP-Link"));
        printf("INFO:testSocketClient: msg written - calling read...\n");
        socketIntf.Read(hSocketIntf, (uint8_t*)buffer, 1024);
        printf("INFO:testSocketClient: read returned:[%s]\n", buffer);
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





