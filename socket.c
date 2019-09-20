/*
*    Socket.c
*/

/* ------------------------------------------------------------ */
/*                Include File Definitions                        */
/* ------------------------------------------------------------ */
#include "socket.h"
/* ------------------------------------------------------------ */
/*                Local Type Definitions                            */
/* ------------------------------------------------------------ */

#define MAXPENDING         1    /* Max connection requests */

/* ------------------------------------------------------------ */
/*                Global Variables                                */
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*                Local Variables                                    */
/* ------------------------------------------------------------ */

static int flag;
static struct sockaddr_in sockaddr;

// For segmenting data being read into the form it was originally sent.
// Data sent in one write statement will be returned via one read statement
static volatile msg_header_t readMsg;

/* ------------------------------------------------------------ */
/*                Forward Declarations                            */
/* ------------------------------------------------------------ */

static uint8_t ComputeChecksum(uint8_t *input, uint32_t length);

/* ------------------------------------------------------------ */
/*                Procedure Definitions                            */
/* ------------------------------------------------------------ */

static uint8_t ComputeChecksum(uint8_t *input, uint32_t length)
{
    int i;
    uint8_t checksum;
    checksum = 0;

    for ( i = 0; i < length; i++ ) {
        //printf("INFO:ComputeChecksum: checksum at top of loop: 0x%x\n", checksum);
        checksum = checksum ^ input[i];
        //printf("INFO:ComputeChecksum: input[%d]: 0x%x, checksum at bottom of loop: 0x%x\n", i, input[i], checksum);
    }

    return checksum;
}

static int OpenServerSocket(uint16_t port)
{
    int hSocket;
    /* Create the TCP socket */
    if ((hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        //printf("->socket() returned fail\n");
        return -1;
    }

    /* Disable the Nagle (TCP No Delay) algorithm */
    flag = 1;
    if (-1 == setsockopt( hSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) ) ) {
        //printf("->setsockopt (nagle)\n");
        return -1;
    }
    /* Set the Keep Alive property */
    flag = 1;
    if (-1 == setsockopt( hSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(flag) ) ) {
        //printf("->setsockopt(keep alive)\n");
        return -1;
    }
    /* Allow the re-use of port numbers to avoid error */
    flag = 1;
    if (-1 == setsockopt( hSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) ) {
        //printf("->setsockopt(port re-use)\n");
        return -1;
    }

    /* Construct the server sockaddr_in structure */
    memset(&sockaddr, 0, sizeof(sockaddr));            /* Clear struct */
    sockaddr.sin_family = AF_INET;                    /* Internet/IP */
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);        /* Incoming addr */
    sockaddr.sin_port = htons(port);                    /* server port */

    /* Bind the server socket */
    if (bind(hSocket, (struct sockaddr *) &sockaddr,
                       sizeof(sockaddr)) < 0) {
        //printf("->bind\n");
        return -1;
    }
    /* Listen on the server socket */
    if (listen(hSocket, MAXPENDING) < 0) {
        //printf("->listen()\n");
        return -1;
    }

    return hSocket;
}

static int AcceptClient(int hSocket)
{
    unsigned int sockaddr_len = sizeof(sockaddr);

    return accept(hSocket, (struct sockaddr *) &sockaddr, &sockaddr_len);
}

static int ConnectToServer(const char* szIPAddress, uint16_t portNum)
{
    int hSocket;

    /* Create the TCP socket */
    if ((hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        return -1;
    }

    /* Disable the Nagle (TCP No Delay) algorithm */
    flag = 1;
    if (-1 == setsockopt( hSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag) ) ) {
        return -1;
    }
    /* Set the Keep Alive property */
    flag = 1;
    if (-1 == setsockopt( hSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(flag) ) ) {
        return -1;
    }

    /* server address */
    sockaddr.sin_family = AF_INET;
    inet_aton(szIPAddress, &sockaddr.sin_addr);
    sockaddr.sin_port = htons(portNum);
    /* connect to the server */
    if ( (connect(hSocket, (struct sockaddr*)&sockaddr, sizeof(sockaddr))) < 0 ) {
        return -1;
    }

    return hSocket;
}

static int32_t Read( int hSocket, uint8_t* buffer, uint32_t bufferLen )
{
    static int32_t recLen = 0;
    static int32_t thisRecLen = 0;
    static uint32_t iBuffer = 0;
    static uint8_t recBuffer[MAX_READ_BUFFER_LEN];
    msg_header_t *msg;
    int i;

    do {
        // Prep our recBuffer with data - either by reading or advancing leftover data from last read
        if ( iBuffer != 0 ) { // if there's leftover data from the last call, process where we left off
            //printf("INFO:socket:Read(): advancing [%d] bytes data residing in the buffer\n", recLen);
            // memcpy has undefined behavior when there are overlapping regions (like this one)
            for ( i = 0; i < recLen; i++, iBuffer++ ) {
                recBuffer[i] = recBuffer[iBuffer];
            }
            iBuffer = 0; // reset
        } else { // call recv to get more data from kernel
            // complete msg not recorded yet (or new one arriving)
            //printf("INFO:socket:Read(): prior to recv call: recLen=%d\n", recLen);
            thisRecLen = recv(hSocket, recBuffer + recLen, MAX_READ_BUFFER_LEN - recLen, 0);
            //printf("INFO:socket:Read(): recv call returned: %d\n", thisRecLen);

            // Valid data check
            if ( thisRecLen == 0 ) {
                //printf("WARN:socket:Read(): recv call returned 0 - returning 0\n");
                return 0;
            } else if ( thisRecLen < 0 ) {
                // trash buffer - won't analyze with subsequent calls
                iBuffer = 0;
                recLen = 0;
                printf("ERROR:socket:Read(): recv returned error - bailing\n");
                return -1;
            }
            recLen = recLen + thisRecLen; // aggregate length with existing contents of buffer
            //printf("INFO:socket:Read(): recLen after agregation: %d\n", recLen);
            /*for ( i = 0; i < recLen; i++ ) {
                printf("    %d->0x%x\n", i, recBuffer[i]);
            }*/
        }

        if ( recLen <= MSG_HEADER_OVERHEAD ) {
            //printf("INFO:socket:Read(): recLen = %d - insufficient for processing - calling recv() again\n", recLen);
            continue;
        }
        //printf("INFO:socket:Read() - have %d bytes, which is enough to process header at least\n", recLen);

        /* Interpret the data that is in the buffer */
        // Look for magic byte
        if ( MSG_MAGIC != recBuffer[0] ) {
            iBuffer = 0;
            recLen = 0;
            //printf("ERROR:socket:Read(): magic byte not in position - bailing\n");
            return -1;
        }

        // Ensure entire data section is present
        msg = (msg_header_t*)(&recBuffer[0]);
        if ( msg->length > (recLen - MSG_HEADER_OVERHEAD) ) { // entire header is present, but not packet data
            /*printf("INFO:socket:Read(): header length is :%d, but buffer only contains: %d data bytes - calling recv() again\n",
                    msg->length, (recLen - MSG_HEADER_OVERHEAD));*/
            continue;
        }

        // Verify checksum
        if ( msg->checksum != ComputeChecksum((uint8_t*)(&msg->data), msg->length) ) { // checksum failed
            /*printf("ERROR:socket:Read(): checksum failed\n");
            printf("  -> msg->checksum: 0x%x\n", msg->checksum);
            printf("  -> msg->length: %d\n", msg->length);
            // just dump 64 bytes off the top of the buffer. I suspect word alignment issues with memcpy...
            uint8_t *ptr;
            ptr = (uint8_t*)(&recBuffer[0]);
            for ( i = 0; i < recLen; i++, ptr++ ) {
                printf("    recLen dump[%d] -> 0x%x\n",i, *ptr);
            }*/
            iBuffer = 0;
            recLen = 0;
            return -1;
        } else { // Good packet data
            //printf("INFO:socket:Read(): checksum passed - copying %d bytes into output buffer\n", msg->length);
            memcpy(buffer, (uint8_t*)(&msg->data), msg->length);
            // Adjust markers if there is additional data here
            if ( recLen == (msg->length + MSG_HEADER_OVERHEAD) ) {
                //printf("INFO:socket:Read(): recLen was equal to header + msg->length: %d. Resetting index and counter\n", recLen);
                iBuffer = 0; // Ensure reset - should already be zero though
                recLen = 0; // Reset
            } else {
                //printf("INFO:socket:Read(): recLen=%d != header + msg->length: %d\n", recLen, (msg->length + MSG_HEADER_OVERHEAD));
                iBuffer = msg->length + MSG_HEADER_OVERHEAD; // move index past this last message
                recLen = recLen - iBuffer; // from all previously captured data, don't count this last message
            }
            //printf("INFO:socket:Read(): returning %d bytes to caller\n", msg->length);
            return msg->length;
        }
    } while ( true ); // Do until one valid message has been read and returned to caller
}

static int32_t Write ( int hSocket, uint8_t* data, uint32_t len )
{
    uint8_t msgBuffer[MAX_READ_BUFFER_LEN];
    msg_header_t *message;
    int32_t ret;
    //int errnum;

    message = (msg_header_t*)(&msgBuffer[0]);

    // obvious error checking
    if ( (NULL == data) || (MAX_READ_BUFFER_LEN - MSG_HEADER_OVERHEAD < len) ) {
        return -1;
    }

    message->magic = MSG_MAGIC;
    message->length = len;
    message->checksum = ComputeChecksum(data, len);
    memcpy((uint8_t*)(&message->data), data, len);

    ret = send(hSocket, msgBuffer, len + MSG_HEADER_OVERHEAD, 0);
    /*if ( -1 == ret ) {
        errnum = errno;
        //printf("ERROR: socket.send() failed with: [%d] %s\n", errno, strerror(errnum) );
    }*/
    return ret;
}

static void Close ( int hSocket )
{
    close(hSocket);
}

// this is the exported Socket interface
SocketInterface_T socketIntf = {
    OpenServerSocket,
    AcceptClient,
    ConnectToServer,
    Read,
    Write,
    Close
};

