
# microAkka an Akka alike framework in C++ to run on microcontrollers

C++ Actor Framework for Embedded Systems - akka alike

![Alt text](doc/esp8266.jpg?raw=true "ESP8266")
## Target

The purpose is to provide a standard C++ framework for writing message drive actors.

Seen the popularity of the Lightbend Akka framework and its extensive documentation and features, I decided to build this 
framework on the same principles and naming conventions. Also the article :
https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6
was very helpful to understand the inner working of Akka framework.

The intention is that a central brain running on PC or server can manage IOT devices, the communication between both should be transparently
as communicating local between actors. The actor systems can communicate using MQTT.

The MQTT topic and message are based on some conventions to ease integration.
#### Request/Reply
- address : "${actorSystem}" => each device listens to one topic
	- dst/$ {actorSystem} = {"$dst":"${address_dest}","$src":"${address_src}","$cls":"${message-class}","$id":123,"${par1}":"${value1}"}
#### Event / properties 
- addres : "${actorSystem}" => each device broadcast on his own topic and subtopics
	- src/${actorSystem}/${actor}/${Property} = ${value}

## Design decisions
- To reduce resource consumptions in a limited embedded environment some design aspects are different from Akka Java/Scala. 
- actors can share the same mailbox, each mailbox has 1 thread ( MessageDispatcher ) to invoke the actors. On FreeRtos based controllers multiple threads ( aka Tasks ) can be started running dispatchers with multiple mailboxes. On Arduino common platform this will be likely limited to 1 thread.
- C++ has limited introspection facilities, so message classes are put explicitly into the message
- the message passed between actors is in a in-memory serialized form, based on aspects from Xdr ( 4 byte granularity ) and Protobuf ( each element is tagged ). The message is copied and a pointer to this copy is passed on. Since most controllers are already 32 bit word aligned, the Xdr form should speed up data retrieval.
- little attention has been given on stopping actors as in an embedded environment these actors are started once and run forever, so no resource cleanup yet there.
- Unique id's are created based on FNV hashing, when compiler optimization is activated this is executed at compile time and not run-time. These unique id's are used for string references in 16 bit and actor references. These 16 bit hashes speed up comparison and extraction

## Platforms supported
- Linux ( Debian ), should work on all linux versions - repository microAkka
- ESP32 with ESP-IDF - repository akkaEsp32
- ESP8266 with ESP-OPEN-RTOS - repository akkaEsp8266
- NOT YET :
Arduino ( ESP 32 and ESP8266 )


### Prerequisites
- C++ 11 compiler
- MQTT library dependent on platform
- Common repository with platform specifics

### Example
An actor that replies with an increment of a counter

C++ code
```
__________________________________________________ main

int main() {

Sys::init();
Mailbox defaultMailbox("default", 100); // nbr of messages in queue max
MessageDispatcher defaultDispatcher;
ActorSystem actorSystem(Sys::hostname(), defaultDispatcher, defaultMailbox);

ActorRef sender = actorSystem.actorOf<Sender>("sender"); // will also start echo actor
ActorRef system = actorSystem.actorOf<System>("System");
ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
ActorRef mqtt = actorSystem.actorOf<Mqtt>("mqtt", "tcp://limero.ddns.net:1883");
ActorRef bridge = actorSystem.actorOf<Bridge>("bridge",mqtt);

defaultDispatcher.attach(defaultMailbox);
defaultDispatcher.unhandled(bridge.cell());
defaultDispatcher.execute();
__________________________________________________ Echo.cpp

Echo::Echo(va_list args)  {}
Echo::~Echo() {}

Receive& Echo::createReceive() {
	return receiveBuilder()
			.match(PING,
	[this](Envelope& msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
		sender().tell(Msg(PONG)("counter",counter+1),self());
	})
	.build();
}
```

Java / Scala code
```
Coming
```


### Installing

- Download git repo https://github.com/vortex314/microAkka
- Download git repo https://github.com/vortex314/Common
- Download git repo https://github.com/bblanchon/ArduinoJson
	- set env variables and mqtt URL

	```
	Give the example
	```


	End with an example of getting some data out of the system or using it for a little demo

## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Codelite](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Akka doc](https://doc.akka.io/docs/akka/2.5/general/actor-systems.html) - Documentation
* [How Akka Actors work ](https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6) - Used to generate RSS Feeds


## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/vortex314/microAkka/tags).

## Authors

* **Lieven Merckx** - *Initial work* -

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
