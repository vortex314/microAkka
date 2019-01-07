#include "System.h"

#include <System.h>

const MsgClass System::Exit("system/Exit");
const MsgClass System::ConfigRequest("system/ConfigRequest");
const MsgClass System::ConfigReply("system/ConfigReply");

System::System(va_list args) {}
System::~System() {}

void System::preStart() {
//	_propTimer = timers().startPeriodicTimer("propTimer", TimerExpired(), 5000);
}

#ifdef __linux__
#include <sys/sysinfo.h>


Receive& System::createReceive() {
	return receiveBuilder()
	.match(Exit, [](Msg& msg) { exit(0); })
	.match(ConfigRequest, [](Msg& msg) {  })
	.match(ConfigReply, [](Msg& msg) {  })
	.match(Properties(),[this](Msg& msg) {
		INFO(" Properties requested ");
		struct sysinfo info;
		sysinfo(&info);
		sender().tell(replyBuilder(msg)
		              ("build",__DATE__ " " __TIME__)
		              ("cpu","x86_64")
		              ("procs",get_nprocs())
		              ("upTime",info.uptime*1000)
		              ("ram",info.totalram)
		              ("hostname",Sys::hostname()),self());
	})
	.build();
}
#endif

#ifdef ARDUINO
#include <Arduino.h>
#include <Esp.h>
Receive& System::createReceive() {
	return receiveBuilder()
	.match(Exit, [](Msg& msg) { exit(0); })
	.match(ConfigRequest, [](Msg& msg) {  })
	.match(ConfigReply, [](Msg& msg) {  })
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
	.build();
}
#endif
