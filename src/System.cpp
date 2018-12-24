#include "System.h"

#include <System.h>
#include <sys/sysinfo.h>

const MsgClass System::Exit("Exit");
const MsgClass System::ConfigRequest("ConfigRequest");
const MsgClass System::ConfigReply("ConfigReply");

System::System(va_list args) {}
System::~System() {}



Receive& System::createReceive() {
	return receiveBuilder()
	.match(Exit, [](Envelope& msg) { exit(0); })
	.match(ConfigRequest, [](Envelope& msg) { exit(0); })
	.match(ConfigReply, [](Envelope& msg) { exit(0); })
	.match(Properties,[this](Envelope& msg) {
		INFO(" Properties requested ");
		struct sysinfo info;
		sysinfo(&info);
		Msg m(PropertiesReply,40);
		m("cpu","x86_64");
		m("procs",get_nprocs());
		m("upTime",info.uptime*1000);
		m("ram",info.totalram);
		sender().tell(self(),m);
	})
	.build();
}
