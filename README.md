# ZeroSDN Controller

Zero Software Defined Networking(ZSDN) is a component based SDN controller. The software is fully distributed which means there are no central instances that control the whole network. 
Instead it consists of multiple modules that are connected by a ZMQ (see http://zeromq.org) message BUS. The communication between modules is handled with google protobuffers. Currently ZeroSDN supports OpenFlow versions 1.0 und 1.3.

The excecution of each module is controlled by an instance of the Zero Module Framework (ZMF). ZMF is a framework for distrubed, language independent module execution. Currently there are two implementations of ZMF: ZMF for C++ https://github.com/zeroSDN/ZMF and JMF for Java https://github.com/zeroSDN/JMF.


## Startup

Pull the repository and run init-zsdn.sh.


## Build

ZeroSDN uses CMake as build system. You can run *build-all.sh* to generate the library.


## Licence

ZeroSDN is licenced under the Apache License Version 2.0, see LICENCE.md


## Support 

You can find the ZeroSDN documentation in our github wiki at https://github.com/zeroSDN/ZSDN-Controller/wiki

For ZMF/JMF releated questions please visit the ZMF page https://github.com/zeroSDN/ZMF or the JMF page https://github.com/zeroSDN/JMF

If you have further questions you can find our contact information here: https://github.com/zeroSDN


## Participation

ZeroSDN is an open source project. If you have any bug reports, fixes, features or feature request feel free to create a pull request https://github.com/zeroSDN/ZSDN-Controller/compare or contact us https://github.com/zeroSDN.

We are looking forward to your participation!
