#ifndef NETWORKPROTOCOL_CONNECTIONBENCHMARKWRAPPER_H
#define NETWORKPROTOCOL_CONNECTIONBENCHMARKWRAPPER_H
#include <cstdint>
#include <map>
#include <vector>

#include "ConnectionBenchmark.h"
#include "../Messages/messageObjects.h"


class ConnectionBenchmarkWrapper {

    /**
     * Number of messages sent for each benchmark.
     */
    uint8_t numberMessages;

    /**
     * Number of messages sent for the current device.
     */
    uint8_t numberMessagesSent;

    /**
     * Index of the current device.
     */
    uint8_t currentDeviceIndex;

    /**
     * ID of this device.
     */
    uint8_t id;

    /**
     * Number of devices to benchmark.
     */
    uint8_t numberDevices;

    /**
     * IDs of the devices to benchmark.
     */
    uint8_t **devices;

    /**
     * Benchmarks for the connections tested.
     * Key: ID of the device.
     * Value: Pointer to the connection benchmark object.
     */
    std::map<uint8_t, ConnectionBenchmark*> benchmarks;

public:
    /**
     * Destructor. Destroys device array.
     */
    ~ConnectionBenchmarkWrapper() {
        delete[] this->devices;
    }

    /**
     * Sets up benchmark tests.
     * @param devices IDs of the devices to benchmark.
     * @param numberDevices Number of devices to benchmark.
     * @param numberMessages Number of messages sent for each benchmark.
     * @param timeout Time maximum waited in milliseconds beginning at the current time.
     * @param currentTime Current time.
     * @param id ID of this ID.
     */
    ConnectionBenchmarkWrapper(uint8_t **devices, uint8_t numberDevices, uint8_t numberMessages,
        uint16_t timeout, uint32_t currentTime, uint8_t id) : numberMessages(numberMessages), numberMessagesSent(0),
                                                              currentDeviceIndex(0), id(id),
                                                              numberDevices(numberDevices),
                                                              devices(devices) {
        for (uint8_t i = 0; i < numberDevices; i++) {
            this->benchmarks[(*devices)[i]] = new ConnectionBenchmark(timeout, currentTime);
        }
    }

    /**
     * Checks if the benchmark is finished.
     * @param time Current time.
     * @param msg If the benchmark is not finished yet, this message has to be sent and then deleted.
     * @return True, if benchmark is finished.
     */
    bool update(uint32_t time, Message** msg);

    /**
     * Got a new answer, save it.
     * @param id ID of the benchmarked device.
     * @param rtt Round trip time of the ping.
     */
    void newAnswer(uint8_t id, uint16_t rtt);
};


#endif //NETWORKPROTOCOL_CONNECTIONBENCHMARKWRAPPER_H