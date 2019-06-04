#include "ConfigActor.h"

ConfigActor::ConfigActor() : startTime(0),  _counter(0) {}
ConfigActor::~ConfigActor() {}
MsgClass ConfigActor::Set("set");
MsgClass ConfigActor::Get("get");
MsgClass ConfigActor::Clear("clear");

void ConfigActor::preStart() {
//	config.load();
}

Receive& ConfigActor::createReceive() {
	return receiveBuilder()
	.match(Get, [this](Msg& msg) {
		std::string output;
		config.save(output);
		sender().tell(replyBuilder(msg)("config",output),self());
	})

	.match(Clear,[this](Msg& msg) {
		config.clear();
		sender().tell(replyBuilder(msg)("erc",0),self());
	})

	.match(Set, [this](Msg& msg) {
		std::string ns;
		std::string key;
		std::string value;
		if ( msg.get("namespace",ns) && msg.get("key",key) ) {
			if ( msg.get("value",value) ) { // string
				INFO(" config set %s:%s = %s from %s ",ns.c_str(),key.c_str(),value.c_str(),sender().path());
				config.setNameSpace(ns.c_str());
				config.set(key.c_str(),value);
				config.save();
				sender().tell(replyBuilder(msg)("erc",0),self());
			} //TODO add other types
		} else {
			sender().tell(replyBuilder(msg)("erc",EINVAL),self());
		}
	})

	.match(MsgClass::Properties(), [this](Msg& msg) {sender().tell(replyBuilder(msg)("config","loaded"),self());})

	.build();
}
