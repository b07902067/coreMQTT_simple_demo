#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "source/transport_interface.h"
#include "source/core_mqtt.h"

#define HOST_IP_ADDR    "127.0.0.1"
#define HOST_PORT_NUM   1883
#define NETWORK_BUFFER_SIZE ( 1024U )

#define NANOSECONDS_PER_MILLISECOND    ( 1000000L )  
#define MILLISECONDS_PER_SECOND        ( 1000L ) 


struct NetworkContext{
    int32_t socketfd;
};

void my_socket_connect(NetworkContext_t* pNetworkContext);

int32_t my_Recv(NetworkContext_t* pNetworkContext,
                        void* pBuffer,
                        size_t bytesToRecv);

int32_t my_Send( NetworkContext_t* pNetworkContext,
                        const void* pBuffer,
                        size_t bytesToSend);

uint32_t my_GetTimeMs( void );

static void eventCallback( MQTTContext_t * pMqttContext,
                           MQTTPacketInfo_t * pPacketInfo,
                           MQTTDeserializedInfo_t * pDeserializedInfo );

static void handleIncomingPublish( MQTTPublishInfo_t * pPublishInfo,
                                   uint16_t packetIdentifier );



void my_socket_connect(NetworkContext_t* pNetworkContext){
    pNetworkContext -> socketfd = socket(AF_INET, SOCK_STREAM, 0); // TCP socket

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET; // Inet IpV4
    info.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    info.sin_port = htons(HOST_PORT_NUM);

    int err = connect(pNetworkContext -> socketfd, (struct sockaddr *)&info, sizeof(info)); 
    if(err != 0) printf("[Socket Connect ERROR] Connection error");
    else printf("[Socket Connect SUCCESS] Connection to %s:%d success!!\n", HOST_IP_ADDR, HOST_PORT_NUM);
}

int32_t my_Recv( NetworkContext_t* pNetworkContext, void * pBuffer, size_t bytesToRecv){
    int32_t copy_socketfd = pNetworkContext -> socketfd;
    int32_t bytesReceived = -1, pollStatus = 1;
    struct pollfd pollFds;

    // assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    // assert( pBuffer != NULL );
    // assert( bytesToRecv > 0 );

    /* Get receive timeout from the socket to use as the timeout for #select. */
    // pPlaintextParams = pNetworkContext->pParams;

    /* Initialize the file descriptor.
     * #POLLPRI corresponds to high-priority data while #POLLIN corresponds
     * to any other data that may be read. */
    pollFds.events = POLLIN | POLLPRI;
    pollFds.revents = 0;
    /* Set the file descriptor for poll. */
    pollFds.fd = copy_socketfd;

    /* Speculative read for the start of a payload.
     * Note: This is done to avoid blocking when
     * no data is available to be read from the socket. */
    if( bytesToRecv == 1U )
    {
        /* Check if there is data to read (without blocking) from the socket. */
        pollStatus = poll( &pollFds, 1, 0 );
    }

    if( pollStatus > 0 )
    {
        /* The socket is available for receiving data. */
        bytesReceived = ( int32_t ) recv( copy_socketfd,
                                          pBuffer,
                                          bytesToRecv,
                                          0 );
    }
    else if( pollStatus < 0 )
    {
        /* An error occurred while polling. */
        bytesReceived = -1;
    }
    else
    {
        /* No data available to receive. */
        bytesReceived = 0;
    }

    /* Note: A zero value return from recv() represents
     * closure of TCP connection by the peer. */
    if( ( pollStatus > 0 ) && ( bytesReceived == 0 ) )
    {
        /* Peer has closed the connection. Treat as an error. */
        bytesReceived = -1;
    }
    else if( bytesReceived < 0 )
    {
        // logTransportError( errno );
    }
    else
    {
        /* Empty else MISRA 15.7 */
    }

    return bytesReceived;
}
int32_t my_Send( NetworkContext_t* pNetworkContext, const void* pBuffer, size_t bytesToSend){
    int32_t copy_socketfd = pNetworkContext -> socketfd;
    int32_t bytesSent = -1, pollStatus = -1;
    struct pollfd pollFds;

    // assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    // assert( pBuffer != NULL );
    // assert( bytesToSend > 0 );

    /* Get send timeout from the socket to use as the timeout for #select. */
    // pPlaintextParams = pNetworkContext->pParams;

    /* Initialize the file descriptor. */
    pollFds.events = POLLOUT;
    pollFds.revents = 0;
    /* Set the file descriptor for poll. */
    pollFds.fd = copy_socketfd;

    /* Check if data can be written to the socket.
     * Note: This is done to avoid blocking on send() when
     * the socket is not ready to accept more data for network
     * transmission (possibly due to a full TX buffer). */
    pollStatus = poll( &pollFds, 1, 0 );

    if( pollStatus > 0 )
    {
        /* The socket is available for sending data. */
        bytesSent = ( int32_t ) send( copy_socketfd,
                                      pBuffer,
                                      bytesToSend,
                                      0 );
    }
    else if( pollStatus < 0 )
    {
        /* An error occurred while polling. */
        bytesSent = -1;
    }
    else
    {
        /* Socket is not available for sending data. */
        bytesSent = 0;
    }

    if( ( pollStatus > 0 ) && ( bytesSent == 0 ) )
    {
        /* Peer has closed the connection. Treat as an error. */
        bytesSent = -1;
    }
    else if( bytesSent < 0 )
    {
        // logTransportError( errno );
    }
    else
    {
        /* Empty else MISRA 15.7 */
    }

    return bytesSent;
}


uint32_t my_GetTimeMs( void ){
     int64_t timeMs;
    struct timespec timeSpec;

    /* Get the MONOTONIC time. */
    ( void ) clock_gettime( CLOCK_MONOTONIC, &timeSpec );

    /* Calculate the milliseconds from timespec. */
    timeMs = ( timeSpec.tv_sec * MILLISECONDS_PER_SECOND )
             + ( timeSpec.tv_nsec / NANOSECONDS_PER_MILLISECOND );

    /* Libraries need only the lower 32 bits of the time in milliseconds, since
     * this function is used only for calculating the time difference.
     * Also, the possible overflows of this time value are handled by the
     * libraries. */
    return ( uint32_t ) timeMs;
}

static void eventCallback( MQTTContext_t * pMqttContext,
                           MQTTPacketInfo_t * pPacketInfo,
                           MQTTDeserializedInfo_t * pDeserializedInfo )
{
    uint16_t packetIdentifier;

    assert( pMqttContext != NULL );
    assert( pPacketInfo != NULL );
    assert( pDeserializedInfo != NULL );

    /* Suppress unused parameter warning when asserts are disabled in build. */
    ( void ) pMqttContext;

    packetIdentifier = pDeserializedInfo->packetIdentifier;

    /* Handle incoming publish. The lower 4 bits of the publish packet
     * type is used for the dup, QoS, and retain flags. Hence masking
     * out the lower bits to check if the packet is publish. */
    if( ( pPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH )
    {
        assert( pDeserializedInfo->pPublishInfo != NULL );
        /* Handle incoming publish. */
        handleIncomingPublish( pDeserializedInfo->pPublishInfo, packetIdentifier );
    }
}

static void handleIncomingPublish( MQTTPublishInfo_t * pPublishInfo,
                                   uint16_t packetIdentifier )
{
    // assert( pPublishInfo != NULL );

    ( void ) packetIdentifier;

    /* Process incoming Publish. */
    printf( "[FROM MQTT broker] Incoming QOS : %d\n", pPublishInfo->qos );

    /* Verify the received publish is for the topic we have subscribed to. */
    if( ( pPublishInfo->topicNameLength == strlen("example/wistron") ) &&
        ( 0 == strncmp( "example/wistron",
                        pPublishInfo->pTopicName,
                        pPublishInfo->topicNameLength ) ) )
    {
        printf( "[FROM MQTT broker] Incoming Publish Topic Name: %.*s matches subscribed topic.\n"
                   "Incoming Publish message Packet Id is %u.\n"
                   "Incoming Publish Message : %.*s.\n\n",
                   pPublishInfo->topicNameLength,
                   pPublishInfo->pTopicName,
                   packetIdentifier,
                   ( int ) pPublishInfo->payloadLength,
                   ( const char * ) pPublishInfo->pPayload );
    }
    else
    {
        printf( "Incoming Publish Topic Name: %.*s does not match subscribed topic.",
                   pPublishInfo->topicNameLength,
                   pPublishInfo->pTopicName );
    }
}
