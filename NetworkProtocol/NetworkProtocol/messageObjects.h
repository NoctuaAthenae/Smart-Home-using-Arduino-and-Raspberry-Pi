#ifndef NETWORKPROTOCOL_MESSAGEOBJECTS_H
#define NETWORKPROTOCOL_MESSAGEOBJECTS_H
#include <cstdint>
#include <utility>
#include <vector>
#include <cstring>

#define VERSION 0

/**
 * Base class for all messages.
 */
class Message {
public:
    virtual ~Message() = default;

    /**
     * Timestamp of the sending of the message.
     */
    uint32_t timestamp;

    /**
     * Current version of the protocol.
     */
    uint8_t version;

    /**
     * Receiver of this message.
     */
    uint8_t receiver;

    /**
     * For incoming messages, this is the last device, that handled the message.
     * For outgoing messages, this is the ID of this device.
     */
    uint8_t lastDeviceId;

    /**
     * For incoming messages, this has to be the ID of this device to be received
     * and forwarded or processed by this device.
     * For outgoing messages, this is the ID of the next Device to handle the message.
     */
    uint8_t nextHop;

    /**
     * Saves the message type and group flags.
     * 1 bit Group Flag
     * 1 bit Group Ascending Flag
     * 6 bit message type
     * To get type: shift 2 to the right.
     */
    uint8_t typeAndGroups;

    /**
     * Checksum to check for transmission errors.
     */
    uint8_t checksum;

    /**
     * @return Type of this message.
     */
    virtual uint8_t getType();

    /**
     * @return True, if this message is addressed to a group.
     */
    bool isGroup();

    /**
     * @param set True, if this message is addressed to a group.
     */
    void setGroup(bool set);

    /**
     * Group messages ascend first to the hub before they wander down the tree.
     * @return True, if this message is ascending to the hub.
     */
    bool isGroupAscending();

    /**
     * Group messages ascend first to the hub before they wander down the tree.
     * @param set True, if this message is ascending to the hub.
     */
    void setGroupAscending(bool set);

    /**
     * Checks if the checksum is correct.
     */
    static bool checkChecksum(uint8_t* rawPackage);

    static Message *fromRawBytes(const uint8_t* rawPackage);

    /**
     * Converts the messages to arrays of 32 bytes.
     * For messages larger than 32 bytes, the message is split and each message is added to the returned vector.
     * Since only command messages can be larger than 32 bytes,
     * all other message types return a vector with only one entry.
     * @return Vector with all messages.
     */
    virtual std::vector<uint8_t*> getRawPackages();

    /**
     * Frees the memory of all raw packages of this message.
     * @param packages Address of the vector with all raw packages of this message.
     */
    static void cleanUp(const std::vector<uint8_t*>* packages);

    /**
     * Base constructor for a new Message.
     * @param receiver Receiver of this message.
     * @param lastDeviceId Last device, that handled the message.
     * @param nextHop Next device, that has to handle the message.
     * @param typeAndGroups Message type and group flags.
     */
    Message(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups) {
        this->version = VERSION;
        this->receiver = receiver;
        this->lastDeviceId = lastDeviceId;
        this->nextHop = nextHop;
        this->typeAndGroups = typeAndGroups;
        this->timestamp = 0;
        this->checksum = 0;
    }

protected:
    /**
     * Adds a checksum to the message.
     */
    static void addChecksum(uint8_t* rawPackage);

    /**
     * Calculates the checksum of the given raw Package.
     */
    static uint8_t getChecksum(uint8_t* rawPackage);
};


/**
 * Class for command messages.
 */
class CommandMessage : public Message {
    /**
    * Constructor for command messages.
     * @param receiver Receiver of this message.
     * @param lastDeviceId Last device, that handled the message.
     * @param nextHop Next device, that has to handle the message.
     * @param typeAndGroups Message type and group flags.
     * @param command Command type of the message.
     * @param content Parameters and other content of the command.
     */
    explicit CommandMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t command, std::vector<uint8_t> content)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {

        this->command = command;
        this->content = std::move(content);
    }

    /**
     * Command type of the message.
     */
    uint8_t command;

    /**
     * Parameters and other content of the message.
     */
    std::vector<uint8_t> content;

    /**
     * Converts the messages to arrays of 32 bytes.
     * For messages larger than 32 bytes, the message is split and each message is added to the returned vector.
     * @return Vector with all messages.
     */
    std::vector<uint8_t*> getRawPackages() override;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 0;
    }
};

/**
 * Class for acknowledge messages.
 */
class AcknowledgeMessage : public Message {
public:
    /**
     * Constructor for an acknowledge message.
     * @param receiver Receiver of the message. Always the same as nextHop.
     * @param lastDeviceId Last device, that handled the message.
     * @param typeAndGroups Message type and group flags.
     * @param timestamp Timestamp of the original message.
     */
    explicit AcknowledgeMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t typeAndGroups, uint32_t timestamp)
        : Message(receiver, lastDeviceId, receiver, typeAndGroups) {
        this->timestamp = timestamp;
    }

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 1;
    }
};

/**
 * Class for register messages.
 */
class RegisterMessage : public Message {
public:
    using Message::Message;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 2;
    }
};

/**
 * Class for message, that indicate whether a registration has benn accepted or rejected.
 */
class AcceptRejectMessage : public Message {
public:
    /**
     * Constructor for accept or reject messages.
     * @param receiver Receiver of this message.
     * @param lastDeviceId Last device, that handled the message.
     * @param nextHop Next device, that has to handle the message.
     * @param typeAndGroups Message type and group flags.
     * @param givenId The ID given to the new device.
     * @param isAccept Indicates whether this is an accept or reject message.
     */
    explicit AcceptRejectMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t givenId, bool isAccept)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->givenId = givenId;
        this->isAccept = isAccept;
    }

    /**
     * ID given to the new device.
     */
    uint8_t givenId;

    /**
     * Indicates whether the new device is accepted or rejected.
     */
    bool isAccept;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 3;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

/**
 * Class for ping and ping response messages.
 */
class PingMessage : public Message {
public:
    /**
    * Constructor for ping and ping and response messages.
     * @param receiver Receiver of this message.
     * @param lastDeviceId Last device, that handled the message.
     * @param nextHop Next device, that has to handle the message.
     * @param typeAndGroups Message type and group flags.
     * @param pingId ID of this ping.
     * @param senderId Sender of this ping.
     * @param isResponse Indicates whether this is a ping or a response to one.
     */
    explicit PingMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t pingId, uint8_t senderId, bool isResponse)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->pingId = pingId;
        this->senderId = senderId;
        this->isResponse = isResponse;
    }

    /**
     * ID of this ping.
     */
    uint8_t pingId;

    /**
     * Sender of this ping.
     */
    uint8_t senderId;

    /**
     * Indicates whether this is a ping or a response.
     */
    bool isResponse;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 4;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

/**
 * Class for route creation messages.
 */
class RouteCreationMessage : public Message {
public:
    /**
     * Constructor for route creation messages.
     * @param receiver Receiver of this message.
     * @param lastDeviceId Last device, that handled the message.
     * @param nextHop Next device, that has to handle the message.
     * @param typeAndGroups Message type and group flags.
     * @param newId ID of the new device. 0 if the device has not been been registered before.
     * @param route Route to the new device so far.
     * @param numberHopsInRoute Number of hops in the route so far.
     */
    explicit RouteCreationMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t newId, const uint8_t* route, uint8_t numberHopsInRoute)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->newId = newId;
        for (int i = 0; i < numberHopsInRoute; i++) {
            this->route[i] = route[i];
        }
    }

    /**
     * ID of the new device. 0 if the device has not been been registered before.
     */
    uint8_t newId;

    /**
     * Route to the new device so far.
     */
    uint8_t route[21]{};

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 5;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

/**
 * Class for messages to add or remove devices to/from a group.
 */
class AddRemoveToGroupMessage : public Message {
public:
    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param lastDeviceId Last device, that handled the message.
     * @param nextHop Next device, that has to handle the message.
     * @param typeAndGroups Message type and group flags.
     * @param groupId ID of the group.
     * @param isAddToGroup True if the device has to be added to the given group, false if it has to be removed from it.
     */
    explicit AddRemoveToGroupMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t groupId, bool isAddToGroup)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->groupId = groupId;
        this->isAddToGroup = isAddToGroup;
    }

    /**
     * ID of the group.
     */
    uint8_t groupId;

    /**
     * True if the device has to be added to the given group, false if it has to be removed from it.
     */
    bool isAddToGroup;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 6;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

#endif //NETWORKPROTOCOL_MESSAGEOBJECTS_H