#ifndef NETWORKDEVICE_H
#define NETWORKDEVICE_H
#include <cstdint>
#include <map>
#include <vector>

#include "ConnectionBenchmark/ConnectionBenchmarkWrapper.h"
#include "Discovery.h"
#include "timer.h"
#include "Messages/messageObjects.h"


class NetworkDevice {
    /**
     * ID of this device.
     */
    uint8_t id;

    /**
     * ID of this device's parent.
     */
    uint8_t parent;

    /**
     * ID for the next message sent by this device.
     */
    uint8_t nextID;

    /**
     * True, if this device has been registered in the network.
     */
    bool registered;

    /**
     * IDs of this device's children.
     */
    uint8_t children[4] = {};

    /**
     * The routing table holds the next hop for all descendant nodes. If an ID is not in the routing table,
     * the node can be reached over the parent.
     * Key: ID of the descendant node.
     * Value: ID of the children over which the node can be reached.
     */
    std::map<uint8_t, uint8_t> routingTable;

    /**
     * The temporary routing table holds the next hop for all descendant nodes that only have a temporary ID.
     * Key: Temporary ID of the descendant node.
     * Value: ID of the children over which the node can be reached.
     */
    std::map<uint32_t, uint8_t> tempRoutingTable;

    /**
     * This map holds the starting times of all active pings.
     * Key: ID of the pinged node.
     * Value: Timer for the ping.
     */
    std::map<uint8_t, Timer> pings;

    /**
     * IDs of the groups this device is part of.
     */
    std::vector<uint8_t> groups;

    /**
     * Command of the last message received.
     */
    uint8_t lastCommand;

    /**
     * Target of the last returned ping.
     */
    uint8_t lastPingTarget;

    /**
     * Size of the parameter array of the last message received.
     */
    uint16_t lastParametersSize;

    /**
     * Temporary ID of this device.
     */
    uint32_t tempID;

    /**
     * Round Trip Time of the last ping.
     */
    uint32_t lastPingTime;

    /**
     * Level of the hierarchy this device is at. Hub is zero.
     */
    uint8_t hierarchyLevel;

    /**
     * Parameters of the last message received.
     */
    uint8_t *lastParameters {};

    /**
     * Represents an ongoing discovery.
     */
    Discovery* discovery{};

    /**
     * Wrapper for ongoing benchmarks.
     */
    ConnectionBenchmarkWrapper* benchmark_wrapper;

    /**
     * Timeout for timers.
     */
    uint16_t timeout;

    /**
     * Assembles a command message object and sends it.
     * @param receiver ID of the message's receiver/receiving group.
     * @param command Command of the message.
     * @param group True if the receiver is a group.
     * @param groupAscending True if the message targeted to a group is still ascending.
     * @param data Parameters and other data for the command.
     * @param dataSize Size of the data.
     * @return True if the message has been sent successfully.
     */
    bool _assembleAndSend(uint8_t receiver, uint8_t command, bool group, bool groupAscending, uint8_t* data, uint16_t dataSize);

    /**
     * Sends the given message.
     * @param message The message to be sent.
     * @param sender Sender of the message, if it has been received.
     * @return True if the message has been sent successfully.
     */
    bool _sendInternal(Message *message, uint8_t sender = 0);

    /**
     * Processes the received message.
     * @param message Received message.
     * @param sender Sender of the message.
     * @return True if it is a command message.
     */
    bool _processMessage(Message *message, uint8_t sender);

    /**
     * Checks if this device is in the given group.
     * @param group Group to be checked.
     * @return True if this device is in the given group.
     */
    bool isInGroup(uint8_t group);

    /**
     * Registers the device with the discovered device at the lowest hierarchy level.
     * Discovery pointer must not be null.
     * @return True, if a parent has been found, false if not.
     */
    bool registerDevice();

    /**
     * Starts the benchmark of the devices found by discovery.
     * Discovery pointer must not be null.
     */
    void startBenchmark();

protected:
    /**
     * This method passes the message to the data link layer.
     * @param msg Message to be sent.
     * @param nextHop The next hop on the route.
     * @return True if the message has been sent successfully.
     */
    virtual bool _write(Message *msg, uint8_t nextHop);

    /**
     * This method gets the message from the data link layer. Creates the message object on the heap
     * and writes the address of it into the pointer given.
     * @param msg Message received.
     * @return The sender of the message.
     */
    virtual uint8_t _read(Message **msg);

    /**
     * This method checks at the data link layer if there is a new message.
     * @return True if a new message is available.
     */
    virtual bool _messageAvailable();

    /**
     * This method returns the current time.
     * @return The current time.
     */
    virtual uint32_t _getTime();

    /**
     * This method prints an error caused by the given message on a terminal.
     * @param errCode Error code.
     * @param msg Erroneous message.
     */
    virtual void _printError(uint8_t errCode, const uint8_t *msg);

public:
    virtual ~NetworkDevice() = default;

    /**
     * Initializes the data needed for a connection with the network.
     * Has to wait for a response before being able to send a message.
     * @param id ID of this device. If 0 it gets assigned an ID by the hub. If 1 this device is the hub.
     * @param discoveryTimeout Timeout for discoveries of other devices.
     */
    explicit NetworkDevice(const uint8_t id, uint16_t discoveryTimeout = 1000) : id(id), parent(0), nextID(0),
        registered(false), lastCommand(0), lastPingTarget(0), hierarchyLevel(0), benchmark_wrapper(nullptr),
        lastParametersSize(0), tempID(0), lastPingTime(0) {
        this->children[0] = this->children[1] = this->children[2] = this->children[3] = 0;
        this->discovery = new Discovery(discoveryTimeout, id);
        this->lastParameters = new uint8_t[0];
        this->groups.push_back(0);
    }

    /**
     * Checks if a new message is available and handles all background stuff of the network device.
     * @return True if a new message is available.
     */
    bool update();

    /**
     * Sends a command message.
     * @param receiver ID of the message's receiver. 0 is broadcast.
     * @param command Command of the message.
     * @param data Parameters and other data for the command.
     * @param dataSize Size of the data.
     * @return True if the message has been sent successfully.
     */
    bool send(uint8_t receiver, uint8_t command, uint8_t* data, uint16_t dataSize);

    /**
     * Sends a command message.
     * @param group ID of the message's receiving group.
     * @param command Command of the message.
     * @param data Parameters and other data for the command.
     * @param dataSize Size of the data.
     * @return True if the message has been sent successfully.
     */
    bool sendToGroup(uint8_t group, uint8_t command, uint8_t* data, uint16_t dataSize);

    /**
     * Receives the last message.
     * @param data Parameters and other data for the command. Is allocated on the heap and has to be deleted.
     * @param dataSize Size of the data.
     * @return Command of the message.
     */
    uint8_t receive(uint8_t** data, uint16_t* dataSize);

};



#endif //NETWORKDEVICE_H
