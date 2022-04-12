#include "mytest.h"

static uint8_t buffer[ NETWORK_BUFFER_SIZE ];

int main(int argv, char ** argc){

    /*****************************************************************************
    *                              A. Socket Connect                             *
    *****************************************************************************/
    NetworkContext_t my_networkContext = { 0 };
    my_socket_connect(&my_networkContext);



    /*****************************************************************************
    *                              B. MQTT_Init                                  *
    *****************************************************************************/
    /* Implement Transport Interface */
    TransportInterface_t transport;
    transport.pNetworkContext = &my_networkContext;
    transport.send = my_Send;
    transport.recv = my_Recv;

    /* Create Buffer*/
    MQTTFixedBuffer_t networkBuffer;
    networkBuffer.pBuffer = buffer;
    networkBuffer.size = NETWORK_BUFFER_SIZE;

    
    MQTTContext_t mqttContext = { 0 };
    MQTTStatus_t mqttStatus = MQTT_Init( &mqttContext,
                            &transport,
                            my_GetTimeMs,
                            eventCallback,
                            &networkBuffer );
    printf("[Init] mqtt status : %s\n", MQTT_Status_strerror(mqttStatus));



    /*****************************************************************************
    *                              C. MQTT_Connect                               *
    *****************************************************************************/  
    /* MQTTConnectInfo_t */
    MQTTConnectInfo_t connectInfo;

    connectInfo.cleanSession = true;
    connectInfo.pClientIdentifier = "Wistron_11102820";
    connectInfo.clientIdentifierLength = 16U;

    connectInfo.keepAliveSeconds = 60U;
    connectInfo.pUserName = NULL;
    connectInfo.userNameLength = 0U;
    connectInfo.pPassword = NULL;
    connectInfo.passwordLength = 0U;

    /* Send MQTT CONNECT packet to broker. */
    bool sessionPresent;
    mqttStatus = MQTT_Connect( &mqttContext, &connectInfo, NULL, 10U, &sessionPresent );
    printf("[MQTT Connect] mqtt status : %s\n", MQTT_Status_strerror(mqttStatus));
    


    /*****************************************************************************
    *                              D. Subscribe                                  *
    *****************************************************************************/  
    /* Subscribe to topic */
    MQTTSubscribeInfo_t pGlobalSubscriptionList[1];

    /* Start with everything at 0. */
    ( void ) memset( ( void * ) pGlobalSubscriptionList, 0x00, sizeof( pGlobalSubscriptionList ) );

    /* This example subscribes to only one topic and uses QOS0. */
    pGlobalSubscriptionList[ 0 ].qos = MQTTQoS0;
    pGlobalSubscriptionList[ 0 ].pTopicFilter = "example/wistron";
    pGlobalSubscriptionList[ 0 ].topicFilterLength = strlen("example/wistron");

    /* Generate packet identifier for the SUBSCRIBE packet. */
    uint16_t globalSubscribePacketIdentifier = 0U;
    globalSubscribePacketIdentifier = MQTT_GetPacketId( &mqttContext );

    /* Send SUBSCRIBE packet. */
    mqttStatus = MQTT_Subscribe( &mqttContext,
                                 pGlobalSubscriptionList,
                                 sizeof( pGlobalSubscriptionList ) / sizeof( MQTTSubscribeInfo_t ),
                                 globalSubscribePacketIdentifier );
    printf("[MQTT Subscribe] mqtt status : %s\n", MQTT_Status_strerror(mqttStatus));

    /* Catch subscribe ACK package*/
    mqttStatus = MQTT_ProcessLoop( &mqttContext, 500U );
    printf("[MQTT Receive Subscribe ACK] mqtt status : %s\n", MQTT_Status_strerror(mqttStatus));



    /*****************************************************************************
    *                              E. Publish                                    *
    *****************************************************************************/
    /* Publish to topic */
    MQTTPublishInfo_t publishInfo;

    /* Some fields not used by this demo so start with everything at 0. */
    ( void ) memset( ( void * ) &publishInfo, 0x00, sizeof( publishInfo ) );

    /* This example publishes to only one topic and uses QOS0. */
    publishInfo.qos = MQTTQoS0;
    publishInfo.pTopicName = "example/wistron";
    publishInfo.topicNameLength = strlen("example/wistron");
    publishInfo.pPayload = "Hello, 11102820!";
    publishInfo.payloadLength = strlen("Hello, 11102820!");

    /* Send PUBLISH packet. Packet Id is not used for a QoS0 publish.
     * Hence 0 is passed as packet id. */
    mqttStatus = MQTT_Publish( &mqttContext, &publishInfo, 0U );
    printf("[MQTT Publish] mqtt status : %s\n", MQTT_Status_strerror(mqttStatus));

    mqttStatus = MQTT_ProcessLoop( &mqttContext, 500U );
    printf("[MQTT Receive Publish ACK] mqtt status : %s\n", MQTT_Status_strerror(mqttStatus));

}