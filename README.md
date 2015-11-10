# What is ZeroSDN ?

Zero Software Defined Networking(ZSDN) is a distributed SDN controller. It consists of multiple independent modules that are connected by a messaging middleware, ZMQ (see http://zeromq.org). Currently ZeroSDN supports OpenFlow versions 1.0 und 1.3.

ZeroSDN was developed by 13 Students during a software engineering project at the [Distributed Systems department](https://www.ipvs.uni-stuttgart.de/abteilungen/vs?__locale=en) at the [University of Stuttgart](http://www.uni-stuttgart.de/home/).

# Why another SDN-Controller?

We felt  that many controllers are either _too monolithic_, _too hard to understand_, or _not scalable_ enough.

This is why we created a controller that:

* Can run on **any hardware** (Raspberry Pi, Cloud environment, even on SDN switches)

* Is **language independent** (currently Java and C++ supported)

* Can be easily **understood and extended**. 

* Is **highly modularized**: every functionality in ZSDN is a single artifact running independently, no matter if on the same machine or distributed: **there is no huge monolithic controller instance**.<br> 

* Filters events on the sender side: Using hierarchical topic-based subscriptions, we **avoid unecessary event-delivery**. This includes the Switch itself. If no one wants to receive e.g. UDP packets, the Switch will not even send them.

* **Performs very well**: ZSDN won't play to its advantages when running locally only, however, we were able to perform very well when scaling out (Tested using [CBench](http://archive.openflow.org/wk/index.php/Oflops), Throughput mode):

![ZeroSDN StartUpSelector initial screen](http://alki.square7.de/zsdn/throughput_4_node.png)

## How to install/use ZeroSDN?

It's easy, take a look at https://github.com/zeroSDN/ZSDN-Controller/wiki for more information.

# Support

If you need help, don't hesitate to contact us:
contact.zsdn@gmail.com

# Participate

ZeroSDN is an open source project. You have own ideas or plans concerning ZSDN or found bugs? Share them with us.

We are looking forward to your participation!

# Licence

ZeroSDN is licenced under the Apache License Version 2.0, see LICENCE.md

