//
// Created by artemis on 20/10/2026.
//

#ifndef NETWORKPROTOCOL_TESTNETWORKDEVICE_H
#define NETWORKPROTOCOL_TESTNETWORKDEVICE_H

#include "../networkDevice.h"
#include <chrono>

class TestNetworkDevice: public NetworkDevice {

    std::map<uint8_t, std::vector<uint8_t*>*> *out, *in;

protected:

    bool _write(uint8_t *rawPackage, uint8_t nextHop) override;

    bool _broadcastToNeighbours(uint8_t *rawPackage, uint8_t exclude) override;

    /**
     * This method gets the message from the data link layer. Creates the message object on the heap
     * and writes the address of it into the pointer given.
     * @param msg Message received.
     * @return The sender of the message.
     */
    uint8_t _read(uint8_t **rawPackage) override;

    /**
     * This method checks at the data link layer if there is a new message.
     * @return True if a new message is available.
     */
    bool _messageAvailable() override;

    /**
     * This method returns the current time.
     * @return The current time.
     */
    uint32_t _getTime() override;

    /**
     * This method prints an error caused by the given message on a terminal.
     * @param errCode Error code.
     * @param msg Erroneous message.
     */
    void _printError(uint8_t errCode, const uint8_t *msg) override;

public:

    /**
     * Initializes the data needed for a connection with the network.
     * Has to wait for a response before being able to send a message.
     * @param id ID of this device. If 0 it gets assigned an ID by the hub. If 1 this device is the hub.
     * @param parent ID of the parent.
     * @param timeout Timeout of timers in ms.
     * @param out Queue for outgoing messages.
     * @param in Queue for incoming messages.
     */
    explicit TestNetworkDevice(const uint8_t id, uint8_t parent, uint16_t timeout, std::map<uint8_t, std::vector<uint8_t*>*> *in, std::map<uint8_t, std::vector<uint8_t*>*> *out)
    : NetworkDevice(id, parent, timeout) {
        this->out = out;
        this->in = in;
    }
};


#endif //NETWORKPROTOCOL_TESTNETWORKDEVICE_H
