#include "System.h"

#include <System.h>

const MsgClass System::Exit("exit");

System::System(ActorRef& mqtt)
	: _mqtt(mqtt) {
}
System::~System() {
}

void System::preStart() {
	timers().startPeriodicTimer("propTimer", Msg("propTimer"), 5000);
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Connected));
	eb.subscribe(self(), MessageClassifier(_mqtt, Mqtt::Disconnected));
}
#ifdef __APPLE__
Receive& System::createReceive() {
	return receiveBuilder()
	.match(Exit, [](Msg& msg) {exit(0);})
	.match(MsgClass::Properties(),[this](Msg& msg) {
		INFO(" Properties requested ");
		sender().tell(replyBuilder(msg)
		              ("build",__DATE__ " " __TIME__)
		              ("cpu","x86_64")
		              ("hostname",Sys::hostname()),self());
	})
	.match(MsgClass("propTimer"),[](Msg& msg){
		
	})
	.build();
}
#endif

#ifdef __linux__
#include <sys/sysinfo.h>

Receive& System::createReceive() {
	return receiveBuilder()

	.match(Mqtt::Connected, [](Msg& msg) {INFO(" MQTT CONNECTED ");})

	.match(Mqtt::Disconnected, [](Msg& msg) {INFO(" MQTT DISCONNECTED ");})

	.match(Exit, [](Msg& msg) {exit(0);})

	.match(MsgClass::Properties(), [this](Msg& msg) {
		struct sysinfo info;
		sysinfo(&info);
		sender().tell(replyBuilder(msg)
		              ("build",__DATE__ " " __TIME__)
		              ("cpu","x86_64")
		              ("procs",get_nprocs())
		              ("upTime",(uint64_t)info.uptime*1000)
		              ("ram",(uint64_t)info.totalram)
		              ("hostname",Sys::hostname()),self());
	}).build();
}
#endif

#ifdef ARDUINO
#include <Arduino.h>
#include <Esp.h>
Receive& System::createReceive() {
	return receiveBuilder()
	.match(Exit, [](Msg& msg) {exit(0);})
	.match(ConfigRequest, [](Msg& msg) {})
	.match(ConfigReply, [](Msg& msg) {})
	.match(Properties(),[this](Msg& msg) {
		INFO(" Properties requested ");

		sender().tell(replyBuilder(msg)
		              ("build",__DATE__ " " __TIME__)
		              ("cpu","esp8266")("framework","Arduino")
		              ("freq",ESP.getCpuFreqMHz())
		              ("flash",ESP.getFlashChipSize())
		              ("upTime",Sys::millis())
		              ("hostname",Sys::hostname()),self());
	})
	.match(MsgClass("exit"),[](Msg& msg) {
		exit(-1);
	})
	.build();
}
#endif
