# What is ZeroSDN ?

Zero Software Defined Networking(ZSDN) is a distributed SDN controller. It consists of multiple independent modules that are connected by a messaging middleware, ZMQ (see http://zeromq.org). Currently ZeroSDN supports OpenFlow versions 1.0 und 1.3.

ZeroSDN was developed by 13 students during a software engineering project at the Distributed Systems department (see https://www.ipvs.uni-stuttgart.de/abteilungen/vs?__locale=en) at the University of Stuttgart, Germany.

## Overview

![Overview](http://alki.square7.de/zsdn/MessageBus_modules.png)

# Why yet another SDN-Controller?

We felt that many controllers are either _too monolithic_, _too hard to understand_, or _not scalable enough_. 

This is why we created a controller which does not have these limitations.

### Highly modularized, distributed design

Rather than using a monolithic design, ZeroSDN encapsulates controller functions into modules communicating through the ZeroMQ high-perfomance messaging library. These modules can run on a single host or can be distributed between several hosts. Modules can also be added to or removed from a running controller easily.

### Lightweight

ZeroSDN can run on hosts spanning a large performance range including a simple Raspberry Pi, cloud servers, or even on a switch itself.

### Language independent

Out of the box, ZeroSDN supports modules implemented in Java or C++. However, since ZeroSDN utilizes ZeroMQ as messaging library _(which supports many more languages!)_ support for other languages can be added to ZeroSDN easily.

### Extensive documentation

We made sure to document all functionality thoroughly to facilitate the usage and extension of ZeroSDN.

### Flexible Event Filtering

ZeroSDN uses the publish/subscribe paradigm to filter events. We designed a hierarchical pub/sub schema making it easy for modules to receive just the events that are relevant for the module. Thanks to ZeroMQ, events can be filtered at high speed. Here is a simplified example for Packet-In messages from switches:

_A module subscribed to IPv4 packets will not receive ARP packets, but will receive both UDP and TCP packets._

### High performance

The distributed design of ZeroSDN based on the high-performance messaging library ZeroMQ helps it to scale well with the number of hosts:

![ZeroSDN StartUpSelector initial screen](http://alki.square7.de/zsdn/throughput_4_node.png)
_Tested using Cbench. 16 Switches, throughput mode_

## How to install/use ZeroSDN?

It's easy, take a look at https://github.com/zeroSDN/ZSDN-Controller/wiki for more information.

# Support

If you need help, don't hesitate to contact us:
contact.zsdn@gmail.com

# Participate

ZeroSDN is an open source project. You have own ideas or plans concerning ZSDN or found bugs? Share them with us.

You can create pull requests at https://github.com/zeroSDN/ZSDN-Controller/pulls or contact us at https://github.com/zeroSDN

We are looking forward to your participation!

# Licence

ZeroSDN is licenced under the Apache License Version 2.0, see LICENCE.md

