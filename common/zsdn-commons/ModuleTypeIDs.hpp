#ifndef ZSDN_MODULE_CONSTANTS_H
#define ZSDN_MODULE_CONSTANTS_H

#include <stdint.h>
/**
 * @brief   Contains all known MODULE_TYPE_IDs.
 *
 * @author  Andre Kutzleb
 *
 * @date    10.07.2015
 */
namespace zsdn {
    static const uint16_t MODULE_TYPE_ID_SwitchAdapter =            0x0000;
    static const uint16_t MODULE_TYPE_ID_SwitchRegistryModule =     0x0001;
    static const uint16_t MODULE_TYPE_ID_DeviceModule =             0x0002;
    static const uint16_t MODULE_TYPE_ID_ARPModule =                0x0003;
    static const uint16_t MODULE_TYPE_ID_SimpleForwardingModule =   0x0004;
    static const uint16_t MODULE_TYPE_ID_LinkDiscoveryModule =      0x0005;
    static const uint16_t MODULE_TYPE_ID_TopologyModule =           0x0006;
    static const uint16_t MODULE_TYPE_ID_SwitchAdapterServer =      0x0007;
    static const uint16_t MODULE_TYPE_ID_StatisticsModule =         0x0008;
    static const uint16_t MODULE_TYPE_ID_ForwardingModule =         0x0009;
    static const uint16_t MODULE_TYPE_ID_FlowRegistryModule =       0x000a;
    static const uint16_t MODULE_TYPE_ID_LoggingModule =            0x0010;
    static const uint16_t MODULE_TYPE_ID_DemoModule =               0xfff1;
    static const uint16_t MODULE_TYPE_ID_ExampleModuleA =           0xfff2;
    static const uint16_t MODULE_TYPE_ID_ExampleModuleB =           0xfff3;
} // namespace zsdn

#endif //ZSDN_MODULE_CONSTANTS_H
