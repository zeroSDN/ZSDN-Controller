# What is ZeroSDN ?

Zero Software Defined Networking(ZSDN) is a distributed SDN controller. It consists of multiple modules that are connected by a ZMQ (see http://zeromq.org) message BUS. The communication between modules is handled with google protobuffers. Currently ZeroSDN supports OpenFlow versions 1.0 und 1.3.

The excecution of each module is controlled by an instance of the Zero Module Framework (ZMF). ZMF is a framework for distrubed, language independent module execution. Currently there are two implementations of ZMF: ZMF for C++ https://github.com/zeroSDN/ZMF and JMF for Java https://github.com/zeroSDN/JMF.

ZeroSDN was developed during a student project at the Distributed Systems department at the University of Stuttgart.

# Why another SDN-Controller?

We felt  that many controllers are either _too monolithic_, _too hard to understand_, or _not scalable_ enough.

This is why we created a controller that:

* Can run on **any hardware** (Raspberry Pi, Cloud environment, even on SDN switches themselves)

* Is **language independent** (currently Java and C++ supported)

* Can be easily **understood and extended**. 

* Is **highly modularized**: every functionality in ZSDN is a single artifact running independently, no matter if on the same machine or distributed: **there is no huge monolithic controller instance**.<br> 

* Filters events on the sender side: Using hierarchical topic-based subscriptions, we **avoid unecessary event-delivery**. This includes the Switch itself. If no one wants to receive e.g. UDP packets, the Switch will not even forward them anymore.

* **Performs very well**: ZSDN won't play to its advantages when running locally only, however, we were able to perform very well when scaling out (Tested using [CBench](http://archive.openflow.org/wk/index.php/Oflops), Throughput mode):

![ZeroSDN StartUpSelector initial screen](http://alki.square7.de/zsdn/throughput_4_node.png)

## Detailed Information

Please consult our [Wiki](https://github.com/zeroSDN/ZSDN-Controller/wiki) for detailed information about the controller.


## Licence

ZeroSDN is licenced under the Apache License Version 2.0, see LICENCE.md


## Support 

You can find the ZeroSDN documentation in our github wiki at https://github.com/zeroSDN/ZSDN-Controller/wiki

For ZMF/JMF releated questions please visit the ZMF page https://github.com/zeroSDN/ZMF or the JMF page https://github.com/zeroSDN/JMF

If you have further questions you can find our contact information here: https://github.com/zeroSDN


## Participation

ZeroSDN is an open source project. If you have any bug reports, fixes, features or feature request feel free to create a pull request https://github.com/zeroSDN/ZSDN-Controller/pulls or contact us https://github.com/zeroSDN.

We are looking forward to your participation!
