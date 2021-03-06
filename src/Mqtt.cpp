
#include <Mqtt.h>
#include <sys/types.h>
#include <unistd.h>
#include <Config.h>

enum { PUBLISH=0,SUBSCRIBE,CONNECT,DISCONNECT,LWT } ;
// volatile MQTTAsync_token deliveredtoken;

Mqtt::Mqtt(const char* address) {
	_address = address;
	config.setNameSpace("mqtt");
};
Mqtt::~Mqtt() {}
/*
MsgClass Mqtt::PublishRcvd("Mqtt/Publish");
MsgClass Mqtt::Connected("Mqtt/Connected");
MsgClass Mqtt::Disconnected("Mqtt/Disconnected");
MsgClass Mqtt::Publish("Mqtt/Publish");
MsgClass Mqtt::Subscribe("Mqtt/Subscribe");
*/
MsgClass Mqtt::PublishRcvd("mqttPublishRcvd");
MsgClass Mqtt::Connected("mqttConnected");
MsgClass Mqtt::Disconnected("mqttDisconnected");
MsgClass Mqtt::Publish("mqttPublish");
MsgClass Mqtt::Subscribe("mqttSubscribe");


void Mqtt::preStart() {
	timers().startPeriodicTimer("PUB_TIMER", Msg("pubTimer"), 5000);

	//   context().mailbox(remoteMailbox);
	_conn_opts = MQTTAsync_connectOptions_initializer;

	_clientId = self().path();
	_clientId += "#";
	char str[10];
	int pid = getpid();
	snprintf(str, sizeof(str), "%d", pid);
	_clientId += str;

	MQTTAsync_create(&_client, _address.c_str(), _clientId.c_str(),
	                 MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(_client, this, onConnectionLost, onMessageArrived,
	                       onDeliveryComplete);

	_conn_opts.keepAliveInterval = 600;
	_conn_opts.cleansession = 1;
	_conn_opts.onSuccess = onConnect;
	_conn_opts.onFailure = onConnectFailure;
	_conn_opts.context = this;
	mqttConnect();
}

void Mqtt::mqttConnect() {
	int rc;
	INFO(" connecting to %s", _address.c_str());
	if ((rc = MQTTAsync_connect(_client, &_conn_opts)) != MQTTASYNC_SUCCESS) {
		INFO("Failed to start connect, return code %d", rc);
	}
}

void Mqtt::mqttDisconnect() {
	int rc;
	MQTTAsync_disconnectOptions opt;
	opt.timeout = 10;
	if ((rc = MQTTAsync_disconnect(_client, &opt)) != MQTTASYNC_SUCCESS) {
		INFO("Failed to start connect, return code %d", rc);
	}
}

Receive& Mqtt::createReceive() {
	return receiveBuilder()

	.match(MsgClass("pubTimer"),[this](Msg& msg) {
		std::string topic = "src/";
		topic += context().system().label();
		topic += "/system/alive";
		if (_connected) {
			mqttPublish(topic.c_str(), "true");
		}
	})

	.match(Mqtt::Publish,
	[this](Msg& msg) {
		std::string topic;
		std::string message;
		msg.rewind();
		while ( msg.getNext("topic",topic) && msg.getNext("message",message) ) {
			mqttPublish(topic.c_str(),message.c_str());
		}
	})
	.build();
}


void Mqtt::onConnect(void* context, MQTTAsync_successData* response) {
	INFO("Mqtt onConnect()");
	Mqtt* me = (Mqtt*)context;
	// publish Connected
	Msg msg(Connected);
	msg.src(me->self().id());
	eb.publish(msg);
	me->_connected = true;
	// SUBSCRIBE
	std::string topic = "dst/";
	topic += me->context().system().label();
	me->mqttSubscribe(topic.c_str());
	topic += "/#";
	me->mqttSubscribe(topic.c_str());
}

void Mqtt::onConnectionLost(void* context, char* cause) {
	Mqtt* me = (Mqtt*)context;
	Msg msg(Disconnected);
	msg.src(me->self().id());
	eb.publish(msg);
	me->_connected = false;
	me->_conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	WARN(" connection lost ! cause: %s", cause);

	INFO("Reconnecting");
	me->_conn_opts.keepAliveInterval = 20;
	me->_conn_opts.cleansession = 1;
	me->_conn_opts.onSuccess = onConnect;
	me->_conn_opts.onFailure = onConnectFailure;
	me->_conn_opts.context = me;
	if ((rc = MQTTAsync_connect(me->_client, &me->_conn_opts)) !=
	        MQTTASYNC_SUCCESS) {
		INFO("Failed to start connect, return code %d", rc);
	}
}

void Mqtt::onConnectFailure(void* context,
                            MQTTAsync_failureData* response) {
	//    Mqtt* me = (Mqtt*)context;
	WARN("Connect failed, rc %d", response ? response->code : 0);
}

void Mqtt::onDisconnect(void* context, MQTTAsync_successData* response) {
	Mqtt* me = (Mqtt*)context;
	INFO("Successful disconnection %X", me);
}

void Mqtt::onSend(void* context, MQTTAsync_successData* response) {
	//    Mqtt* me = (Mqtt*)context;
	//   INFO("Message with token value %d onSend", response->token);
}

void Mqtt::onDeliveryComplete(void* context, MQTTAsync_token response) {
	//    Mqtt* me = (Mqtt*)context;
	//    INFO("Message with token value %d onDeliveryComplete", response);
}

void Mqtt::onSubscribeFailure(void* context,
                              MQTTAsync_failureData* response) {
	//    Mqtt* me = (Mqtt*)context;
	WARN("Subscribe failed, rc %d", response ? response->code : 0);
}

void Mqtt::onSubscribe(void* context, MQTTAsync_successData* response) {
	INFO("Subscribe success");
}
// send myself message as this is invoked by another thread

int Mqtt::onMessageArrived(void* context, char* topicName, int topicLen,
                           MQTTAsync_message* message) {
	static bool busy=false;
	if ( !busy ) {
		busy=true;

		Mqtt* me = (Mqtt*)context;
		std::string topic;
		topic.assign(topicName, topicLen);
		std::string msg;
		msg.assign((char*)message->payload, message->payloadlen);
		INFO(" MQTT RXD : %s = %s ", topicName, msg.c_str());
		//   me->self().tell(me-self(),MQTT_PUBLISH_RCVD,"SS",&topic,&msg);
		Msg  pub(PublishRcvd);
		pub("topic", topic);
		pub("message", msg);
		pub.src(me->self().id());
		INFO("%s",pub.toString().c_str());
		eb.publish(pub);
		busy=false;
	} else {
		WARN(" dropped message, I am busy !");
	}
	MQTTAsync_freeMessage(&message);
	MQTTAsync_free(topicName);
	return 1;
}

#define POLY 0x8408
/*
//                                      16   12   5
// this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
// This works out to be 0x1021, but the way the algorithm works
// lets us use 0x8408 (the reverse of the bit pattern).  The high
// bit is always assumed to be set, thus we only use 16 bits to
// represent the 17 bit value.
*/

unsigned short crc16(const char *data_p, unsigned short length)
{
      unsigned char i;
      unsigned int data;
      unsigned int crc = 0xffff;

      if (length == 0)
            return (~crc);

      do
      {
            for (i=0, data=(unsigned int)0xff & *data_p++;
                 i < 8; 
                 i++, data >>= 1)
            {
                  if ((crc & 0x0001) ^ (data & 0x0001))
                        crc = (crc >> 1) ^ POLY;
                  else  crc >>= 1;
            }
      } while (--length);

      crc = ~crc;
      data = crc;
      crc = (crc << 8) | (data >> 8 & 0xff);

      return (crc);
}

void Mqtt::mqttPublish(const char* topic, const char* message) {
	INFO(" MQTT TXD : %s = %s", topic, message);	
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;
	_opts.onSuccess = onSend;
	_opts.context = this;

	pubmsg.payload = (void*)message;
	pubmsg.payloadlen = (int)strlen(message);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	_deliveredtoken = 0;

	if ((rc = MQTTAsync_sendMessage(_client, topic, &pubmsg, &_opts)) !=
	        MQTTASYNC_SUCCESS) {
		INFO("Failed to start sendMessage, return code %d", rc);
		mqttDisconnect();
		mqttConnect();
	}
}

void Mqtt::mqttSubscribe(const char* topic) {
	INFO("Subscribing to topic %s for client %s using QoS%d", topic,
	     _clientId.c_str(), QOS);
	_opts.onSuccess = onSubscribe;
	_opts.onFailure = onSubscribeFailure;
	_opts.context = this;

	_deliveredtoken = 0;
	int rc;

	if ((rc = MQTTAsync_subscribe(_client, topic, QOS, &_opts)) !=
	        MQTTASYNC_SUCCESS) {
		INFO("Failed to start subscribe, return code %d", rc);
	}
}
