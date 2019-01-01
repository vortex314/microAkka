#include "ConfigActor.h"

ConfigActor::ConfigActor(va_list args) : startTime(0),  _counter(0) {}
ConfigActor::~ConfigActor() {}
MsgClass ConfigActor::Set("set");
MsgClass ConfigActor::Get("get");
MsgClass ConfigActor::Clear("clear");

void ConfigActor::preStart() {
	config.load();
}

Receive& ConfigActor::createReceive() {
	return receiveBuilder()
	.match(Get, [this](Envelope& msg) {
		std::string output;
		config.print(output);
		sender().tell(msg.reply()("config",output),self());
	})
	.match(Clear,[this](Envelope& msg) {
		config.clear();
		sender().tell(msg.reply()("erc",0),self());
	})
	.match(Set, [this](Envelope& msg) {
		std::string ns;
		std::string key;
		std::string value;
		if ( msg.get("namespace",ns)==0 && msg.get("key",key)==0 ) {
			if ( msg.get("value",value)==0 ) { // string
				config.setNameSpace(ns.c_str());
				config.set(key.c_str(),value);
				config.save();
				sender().tell(msg.reply()("erc",0),self());
			} //TODO add other types
		} else {
			sender().tell(msg.reply()("erc",EINVAL),self());
		}
	})

	.build();
}
