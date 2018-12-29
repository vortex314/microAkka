#include "System.h"

#include <System.h>
#include <sys/sysinfo.h>

const MsgClass System::Exit("Exit");
const MsgClass System::ConfigRequest("ConfigRequest");
const MsgClass System::ConfigReply("ConfigReply");

System::System(va_list args) {}
System::~System() {}

void System::preStart() {
	_propTimer = timers().startPeriodicTimer("propTimer", TimerExpired(), 5000);
}


Receive& System::createReceive() {
	return receiveBuilder()
	.match(Exit, [](Envelope& msg) { exit(0); })
	.match(ConfigRequest, [](Envelope& msg) { exit(0); })
	.match(ConfigReply, [](Envelope& msg) { exit(0); })
	.match(Properties(),[this](Envelope& msg) {
		INFO(" Properties requested ");
		struct sysinfo info;
		sysinfo(&info);

		sender().tell(Msg(PropertiesReply())
		              ("cpu","x86_64")
		              ("procs",get_nprocs())
		              ("upTime",info.uptime*1000)
		              ("ram",info.totalram),self());
	})
	.build();
}
