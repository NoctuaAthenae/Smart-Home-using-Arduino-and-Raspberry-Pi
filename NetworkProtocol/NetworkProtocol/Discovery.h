#ifndef DISCOVERY_H
#define DISCOVERY_H
#include <cstdint>
#include <map>
#include <vector>

#include "timer.h"
#include "Messages/messageObjects.h"


class Discovery {

    /**
     * ID of the device to ping next.
     */
    uint8_t nextDeviceToPing;

    /**
     * Is the sending of the pings finished?
     */
    bool pingFinished = false;

    /**
     * Timer for the discovery timeout.
     */
    Timer timer;

    /**
     * ID of this device.
     */
    uint8_t id;

public:

    /**
     * IDs of the found device and their level in the hierarchy.
     */
    std::vector<std::pair<uint8_t, uint8_t>> foundDevices;

    /**
     * A new discovery is started.
     * @param discoveryWaiting The time in ms how long the discovery waits for answers.
     * @param deviceId ID of this device.
     */
    explicit Discovery(uint16_t discoveryWaiting, uint8_t deviceId) : timer(discoveryWaiting), id(deviceId) {
        this->nextDeviceToPing = 0;
    }

    /**
     * Checks if the discovery is finished.
     * @param time The current time.
     * @param msg Message has to be sent for the discovery.
     * @return True, if discovery is finished.
     */
    bool update(uint32_t time, Message** msg);

    /**
     * Got a new answer, save it.
     * @param id ID of the device, that answered.
     * @param level Level in the hierarchy of the device.
     */
    void newAnswer(uint8_t id, uint8_t level);

};



#endif //DISCOVERY_H
