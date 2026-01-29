#ifndef NETWORKPROTOCOL_NETWORKHUB_H
#define NETWORKPROTOCOL_NETWORKHUB_H
#include "networkDevice.h"


class NetworkHub : NetworkDevice {
    public:

    /**
     * Initializes the hub of the network.
     * @param pingTimeout Timeout for ping of other devices while registration of a new device.
     */
    explicit NetworkHub(uint16_t pingTimeout) : NetworkDevice(0, pingTimeout) {}
};


#endif //NETWORKPROTOCOL_NETWORKHUB_H