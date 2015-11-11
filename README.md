# What is ZeroSDN ?

Zero Software Defined Networking(ZSDN) is a distributed SDN controller. It consists of multiple independent modules that are connected by a messaging middleware, ZMQ (see http://zeromq.org). Currently ZeroSDN supports OpenFlow versions 1.0 und 1.3.

ZeroSDN was developed by 13 students during a software engineering project at the Distributed Systems department (see https://www.ipvs.uni-stuttgart.de/abteilungen/vs?__locale=en) at the University of Stuttgart, Germany.

## Overview

![Overview](http://alki.square7.de/zsdn/MessageBus_modules.png)

# Why yet another SDN-Controller?

We felt that many controllers are either _too monolithic_, _too hard to understand_, or _not scalable enough_. 

This is why we created a controller that:

**Is highly modularized**

Every functionality in ZeroSDN is a single artifact running independently, no matter if on the same machine or distributed; there is no huge monolithic controller instance.

**Can run on any hardware**

We deployed the full controller on a single Raspberry Pi, in a cloud environment and even directly on physical SDN switches.

**Is language independent**

Currently supported languages are Java and C++.

**Can be easily understood and extended**

We made sure to document all functionality thoroughly.

**Avoids unecessary event-delivery**

ZeroSDN filters events at sending modules using hierarchical topic-based publish/subscribe. 
If, for example, no module is subscribed to UDP packets from the network, the switches will not even attempt to deliver them.

**Performs very well**

While ZeroSDN can be run locally on one machine without a problem, it really plays to its advantage once distributed:

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

