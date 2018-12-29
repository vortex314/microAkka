
# microAkka an Akka alike framework in C++ to run on microcontrollers

C++ Actor Framework for Embedded Systems - akka alike

## Target

The purpose is to provide a standard C++ framework for writing message drive actors.

	Seen the popularity of the Lightbend Akka framework and its extensive documentation and features, I decided to build this
framework on the same principles and naming conventions. Also the article :
https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6
	was very helpful to understand the inner working of Akka framework.

	The intention is that a central brain running on PC can manage IOT devices, the communication between both should be transparently
	as communicating local between actors. The actor systems can communicate using MQTT.

	The MQTT protocol is based on certain conventions to support serialization.
- address : "${actorSystem}/${actor}"
	- dst/<$ {actorSystem} = ["${address_dest}","${address_src}",$ {message-class},id-int,.....]

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

		ActorRef sender = actorSystem.actorOf<Sender>("sender");
		ActorRef system = actorSystem.actorOf<System>("System");
		ActorRef nnPid = actorSystem.actorOf<NeuralPid>("neuralPid");
		ActorRef mqtt = actorSystem.actorOf<Mqtt>("mqtt", "tcp://limero.ddns.net:1883");
		ActorRef bridge = actorSystem.actorOf<Bridge>("bridge",mqtt);

		defaultDispatcher.attach(defaultMailbox);
		defaultDispatcher.unhandled(bridge.cell());
		defaultDispatcher.execute();
		__________________________________________________ Echo.cpp
#include <Echo.h>

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
