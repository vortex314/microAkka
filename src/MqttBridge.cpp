#include <MqttBridge.h>

int finished = 0;

volatile MQTTAsync_token deliveredtoken;

MqttBridge::MqttBridge(){
};
MqttBridge::~MqttBridge()
{
}

void MqttBridge::preStart()
{
	context().mailbox(remoteMailbox);
    MQTTAsync client;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTAsync_setCallbacks(client, NULL, connlost, NULL, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    if((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
	printf("Failed to start connect, return code %d\n", rc);
	exit(EXIT_FAILURE);
    }

    printf("Waiting for publication of %s\n"
           "on topic %s for client with ClientID: %s\n",
        PAYLOAD, TOPIC, CLIENTID);
}
Receive& MqttBridge::createReceive()
{
    return receiveBuilder()
        .match(AnyClass,
            [this](Envelope& msg) {
	        INFO(" MqttBridge received message ");
	        //			INFO(" DO_ECHO called ");
	        //	            const char* dst = msg.receiver.id().label();
	        // topic = 'dst/'+
	        // start JSON Array
	        // add sender label
	        // add id
	        // add msgClass
	        // add other elements from Cbor payload
	        // mqtt publish
            })
        .build();
}

void MqttBridge::connlost(void* context, char* cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc;

    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);

    printf("Reconnecting\n");
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
	printf("Failed to start connect, return code %d\n", rc);
	finished = 1;
    }
}

void MqttBridge::onDisconnect(void* context, MQTTAsync_successData* response)
{
    printf("Successful disconnection\n");
    finished = 1;
}

void MqttBridge::onSend(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    printf("Message with token value %d delivery confirmed\n", response->token);

    opts.onSuccess = onDisconnect;
    opts.context = client;

    if((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS) {
	printf("Failed to start sendMessage, return code %d\n", rc);
	exit(EXIT_FAILURE);
    }
}

void MqttBridge::onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Connect failed, rc %d\n", response ? response->code : 0);
    finished = 1;
}

void MqttBridge::onConnect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    printf("Successful connection\n");

    opts.onSuccess = onSend;
    opts.context = client;

    pubmsg.payload = (void*)PAYLOAD;
    pubmsg.payloadlen = (int)strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    deliveredtoken = 0;

    if((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
	printf("Failed to start sendMessage, return code %d\n", rc);
	exit(EXIT_FAILURE);
    }
}
