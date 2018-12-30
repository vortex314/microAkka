---


---

<h1 id="microakka-an-akka-alike-framework-in-c-to-run-on-microcontrollers">microAkka an Akka alike framework in C++ to run on microcontrollers</h1>
<p>C++ Actor Framework for Embedded Systems - akka alike</p>
<img src="doc/ESP8266.jpg" height="100">
<img src="doc/ESP32.png" height="100">
<h2 id="target">Target</h2>
<p>The purpose is to provide a  C++ framework for writing message driven actors.</p>
<p>Seen the popularity of the Lightbend Akka framework and its extensive documentation and features, I decided to build this<br>
framework on the same principles and naming conventions. Saves me some time on documenting my own stuff. ;-)</p>
<p>The article :<br>
<a href="https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6">https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6</a><br>
was very helpful to understand the inner working of Akka framework.</p>
<p>The intention is that a central brain running on PC or server can manage IOT devices, the communication between both should be transparently<br>
as communicating local between actors. The actor systems can communicate using MQTT.</p>
<p>The MQTT topic and message are based on some conventions to ease integration.</p>
<h4 id="requestreply">Request/Reply</h4>
<ul>
<li>address : “${actorSystem}” =&gt; each device listens to one topic
<ul>
<li>dst/$ {actorSystem} = {"<span class="katex--inline"><span class="katex"><span class="katex-mathml"><math><semantics><mrow><mi>d</mi><mi>s</mi><mi>t</mi><mi mathvariant="normal">&amp;quot;</mi><mo>:</mo><mi mathvariant="normal">&amp;quot;</mi></mrow><annotation encoding="application/x-tex">dst&amp;quot;:&amp;quot;</annotation></semantics></math></span><span class="katex-html" aria-hidden="true"><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord mathit">d</span><span class="mord mathit">s</span><span class="mord mathit">t</span><span class="mord">"</span><span class="mspace" style="margin-right: 0.277778em;"></span><span class="mrel">:</span><span class="mspace" style="margin-right: 0.277778em;"></span></span><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord">"</span></span></span></span></span>{address_dest}","<span class="katex--inline"><span class="katex"><span class="katex-mathml"><math><semantics><mrow><mi>s</mi><mi>r</mi><mi>c</mi><mi mathvariant="normal">&amp;quot;</mi><mo>:</mo><mi mathvariant="normal">&amp;quot;</mi></mrow><annotation encoding="application/x-tex">src&amp;quot;:&amp;quot;</annotation></semantics></math></span><span class="katex-html" aria-hidden="true"><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord mathit">s</span><span class="mord mathit" style="margin-right: 0.02778em;">r</span><span class="mord mathit">c</span><span class="mord">"</span><span class="mspace" style="margin-right: 0.277778em;"></span><span class="mrel">:</span><span class="mspace" style="margin-right: 0.277778em;"></span></span><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord">"</span></span></span></span></span>{address_src}","<span class="katex--inline"><span class="katex"><span class="katex-mathml"><math><semantics><mrow><mi>c</mi><mi>l</mi><mi>s</mi><mi mathvariant="normal">&amp;quot;</mi><mo>:</mo><mi mathvariant="normal">&amp;quot;</mi></mrow><annotation encoding="application/x-tex">cls&amp;quot;:&amp;quot;</annotation></semantics></math></span><span class="katex-html" aria-hidden="true"><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord mathit">c</span><span class="mord mathit" style="margin-right: 0.01968em;">l</span><span class="mord mathit">s</span><span class="mord">"</span><span class="mspace" style="margin-right: 0.277778em;"></span><span class="mrel">:</span><span class="mspace" style="margin-right: 0.277778em;"></span></span><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord">"</span></span></span></span></span>{message-class}","<span class="katex--inline"><span class="katex"><span class="katex-mathml"><math><semantics><mrow><mi>i</mi><mi>d</mi><mi mathvariant="normal">&amp;quot;</mi><mo>:</mo><mn>123</mn><mo separator="true">,</mo><mi mathvariant="normal">&amp;quot;</mi></mrow><annotation encoding="application/x-tex">id&amp;quot;:123,&amp;quot;</annotation></semantics></math></span><span class="katex-html" aria-hidden="true"><span class="base"><span class="strut" style="height: 0.69444em; vertical-align: 0em;"></span><span class="mord mathit">i</span><span class="mord mathit">d</span><span class="mord">"</span><span class="mspace" style="margin-right: 0.277778em;"></span><span class="mrel">:</span><span class="mspace" style="margin-right: 0.277778em;"></span></span><span class="base"><span class="strut" style="height: 0.88888em; vertical-align: -0.19444em;"></span><span class="mord">1</span><span class="mord">2</span><span class="mord">3</span><span class="mpunct">,</span><span class="mspace" style="margin-right: 0.166667em;"></span><span class="mord">"</span></span></span></span></span>{par1}":"${value1}"}</li>
</ul>
</li>
</ul>
<h4 id="event--properties">Event / properties</h4>
<ul>
<li>addres : “${actorSystem}” =&gt; each device broadcast on his own topic and subtopics
<ul>
<li>src/<span class="katex--inline"><span class="katex"><span class="katex-mathml"><math><semantics><mrow><mrow><mi>a</mi><mi>c</mi><mi>t</mi><mi>o</mi><mi>r</mi><mi>S</mi><mi>y</mi><mi>s</mi><mi>t</mi><mi>e</mi><mi>m</mi></mrow><mi mathvariant="normal">/</mi></mrow><annotation encoding="application/x-tex">{actorSystem}/</annotation></semantics></math></span><span class="katex-html" aria-hidden="true"><span class="base"><span class="strut" style="height: 1em; vertical-align: -0.25em;"></span><span class="mord"><span class="mord mathit">a</span><span class="mord mathit">c</span><span class="mord mathit">t</span><span class="mord mathit">o</span><span class="mord mathit" style="margin-right: 0.02778em;">r</span><span class="mord mathit" style="margin-right: 0.05764em;">S</span><span class="mord mathit" style="margin-right: 0.03588em;">y</span><span class="mord mathit">s</span><span class="mord mathit">t</span><span class="mord mathit">e</span><span class="mord mathit">m</span></span><span class="mord">/</span></span></span></span></span>{actor}/${Property} = ${value}</li>
</ul>
</li>
</ul>
<h2 id="design-decisions">Design decisions</h2>
<ul>
<li>The interface is close to the Java API of Akka as C++ translation was easier compared to the Scala interface.</li>
<li>To reduce resource consumptions in a limited embedded environment some design aspects are different from Akka Java/Scala.</li>
<li>actors can share the same mailbox, each mailbox has 1 thread ( MessageDispatcher ) to invoke the actors. On FreeRtos based controllers multiple threads ( aka Tasks ) can be started running dispatchers with multiple mailboxes. On Arduino common platform this will be likely limited to 1 thread.</li>
<li>C++ has limited introspection facilities, so message classes are put explicitly into the message</li>
<li>the message passed between actors is in a in-memory serialized form, based on aspects from Xdr ( 4 byte granularity ) and Protobuf ( each element is tagged ). The message is copied and a pointer to this copy is passed on. Since most controllers are already 32 bit word aligned, the Xdr form should speed up data retrieval.</li>
<li>little attention has been given on stopping actors as in an embedded environment these actors are started once and run forever, so no resource cleanup yet there.</li>
<li>Unique id’s are created based on FNV hashing, when compiler optimization is activated this is executed at compile time and not run-time. These unique id’s are used for string references in 16 bit and actor references. These 16 bit hashes speed up comparison and extraction</li>
</ul>
<h2 id="platforms-supported">Platforms supported</h2>
<ul>
<li>Linux ( Debian ), should work on all linux versions - repository microAkka</li>
<li>ESP32 with ESP-IDF - repository akkaEsp32</li>
<li>ESP8266 with ESP-OPEN-RTOS - repository akkaEsp8266</li>
<li>NOT YET :<br>
Arduino ( ESP 32 and ESP8266 )</li>
</ul>
<h3 id="prerequisites">Prerequisites</h3>
<ul>
<li>C++ 11 compiler</li>
<li>MQTT library dependent on platform</li>
<li>Common repository with platform specifics</li>
</ul>
<h3 id="example">Example</h3>
<p>An actor that replies with an increment of a counter</p>
<p>C++ code</p>
<pre><code>__________________________________________________ main

int main() {

Sys::init();
Mailbox defaultMailbox("default", 100); // nbr of messages in queue max
MessageDispatcher defaultDispatcher;
ActorSystem actorSystem(Sys::hostname(), defaultDispatcher, defaultMailbox);

ActorRef sender = actorSystem.actorOf&lt;Sender&gt;("sender"); // will also start echo actor
ActorRef system = actorSystem.actorOf&lt;System&gt;("System");
ActorRef nnPid = actorSystem.actorOf&lt;NeuralPid&gt;("neuralPid");
ActorRef mqtt = actorSystem.actorOf&lt;Mqtt&gt;("mqtt", "tcp://limero.ddns.net:1883");
ActorRef bridge = actorSystem.actorOf&lt;Bridge&gt;("bridge",mqtt);

defaultDispatcher.attach(defaultMailbox);
defaultDispatcher.unhandled(bridge.cell());
defaultDispatcher.execute();
__________________________________________________ Echo.cpp

Echo::Echo(va_list args)  {}
Echo::~Echo() {}

Receive&amp; Echo::createReceive() {
	return receiveBuilder()
			.match(PING,
	[this](Envelope&amp; msg) {
		uint32_t counter;
		assert(msg.get("counter", counter)==0);
		sender().tell(Msg(PONG)("counter",counter+1),self());
	})
	.build();
}
</code></pre>
<p>Java / Scala code</p>
<pre><code>Coming
</code></pre>
<h3 id="installing">Installing</h3>
<ul>
<li>
<p>Download git repo <a href="https://github.com/vortex314/microAkka">https://github.com/vortex314/microAkka</a></p>
</li>
<li>
<p>Download git repo <a href="https://github.com/vortex314/Common">https://github.com/vortex314/Common</a></p>
</li>
<li>
<p>Download git repo <a href="https://github.com/bblanchon/ArduinoJson">https://github.com/bblanchon/ArduinoJson</a></p>
<ul>
<li>set env variables and mqtt URL</li>
</ul>
<pre><code>Give the example
</code></pre>
<p>End with an example of getting some data out of the system or using it for a little demo</p>
</li>
</ul>
<h2 id="running-the-tests">Running the tests</h2>
<p>Explain how to run the automated tests for this system</p>
<h3 id="break-down-into-end-to-end-tests">Break down into end to end tests</h3>
<p>Explain what these tests test and why</p>
<pre><code>Give an example
</code></pre>
<h3 id="and-coding-style-tests">And coding style tests</h3>
<p>Explain what these tests test and why</p>
<pre><code>Give an example
</code></pre>
<h2 id="deployment">Deployment</h2>
<p>Add additional notes about how to deploy this on a live system</p>
<h2 id="built-with">Built With</h2>
<ul>
<li><a href="http://www.dropwizard.io/1.0.2/docs/">Codelite</a> - The web framework used</li>
<li><a href="https://doc.akka.io/docs/akka/2.5/general/actor-systems.html">Akka doc</a> - Documentation</li>
<li><a href="https://medium.com/@unmeshvjoshi/how-akka-actors-work-b0301ec269d6">How Akka Actors work </a> - Used to generate RSS Feeds</li>
</ul>
<h2 id="versioning">Versioning</h2>
<p>We use <a href="http://semver.org/">SemVer</a> for versioning. For the versions available, see the <a href="https://github.com/vortex314/microAkka/tags">tags on this repository</a>.</p>
<h2 id="authors">Authors</h2>
<ul>
<li><strong>Lieven Merckx</strong> - <em>Initial work</em> -</li>
</ul>
<h2 id="license">License</h2>
<p>This project is licensed under the MIT License - see the <a href="LICENSE.md">LICENSE.md</a> file for details</p>

