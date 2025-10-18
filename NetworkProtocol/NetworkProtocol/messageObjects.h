#ifndef NETWORKPROTOCOL_MESSAGEOBJECTS_H
#define NETWORKPROTOCOL_MESSAGEOBJECTS_H
#include <chrono>
#include <cstdint>
#include <cstring>
#include <vector>
#include <functional>

#define NETWORKPROTOCOL_VERSION 0
#define COMMAND_METADATA_SLOTS 7
#define COMMAND_SLOTS (32 - COMMAND_METADATA_SLOTS)
#define FIRST_COMMAND_SLOTS (COMMAND_SLOTS - 2)
#define SLOT_COUNT(i) (FIRST_COMMAND_SLOTS + COMMAND_SLOTS * (i - 1))

/**
 * Base class for all messages.
 */
class Message {
public:
    virtual ~Message() = default;

    /**
     * Current version of the protocol.
     */
    uint8_t version;

    /**
     * Receiver of this message.
     */
    uint8_t receiver;

    /**
     * Saves the message type and group flags.
     * 1 bit Group Flag
     * 1 bit Group Ascending Flag
     * 6 bit message type
     * To get type: shift 2 to the right.
     * Type is hardcoded in the message class.
     * Type is still saved for received messages, but not used.
     */
    uint8_t typeAndGroups;

    /**
     * @return Type of this message.
     */
    virtual uint8_t getType() {
        return 0;
    };

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
     * Uses the given raw package to create a message object. The message object has to be deleted.
     * @param rawPackage The raw package of the message.
     * @return The message object
     */
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
     * Base constructor for a received Message.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     */
    Message(uint8_t receiver, uint8_t typeAndGroups) {
        this->version = NETWORKPROTOCOL_VERSION;
        this->receiver = receiver;
        this->typeAndGroups = typeAndGroups;
    }

    /**
     * Base constructor for a new Message.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     */
    Message(uint8_t receiver, bool group, bool groupAscending) {
        this->version = NETWORKPROTOCOL_VERSION;
        this->receiver = receiver;
        this->typeAndGroups = 0;
        setGroup(group);
        setGroupAscending(groupAscending);
    }
};


/**
 * Class for command messages.
 */
class CommandMessage : public Message {
public:
    ~CommandMessage() {
        delete[] content;
    };

    /**
     * Constructor for command messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param command Command type of the message.
     * @param messageID ID of the message.
     * @param command Command type of the message.
     * @param content Parameters and other content of the command. Allocated on the heap.
     * Is deleted when this message is deleted.
     * @param contentSize Size of the content.
     */
    explicit CommandMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t command,
        uint16_t messageID, uint8_t origin, uint8_t* content, uint16_t contentSize)
        : Message(receiver, typeAndGroups),
            command(command), messageID(messageID), origin(origin), content(content), contentSize(contentSize) {}

    /**
     * Constructor for command messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param command Command type of the message.
     * @param messageID ID of the message.
     * @param origin Device that created the message.
     * @param content Parameters and other content of the command. Allocated on the heap.
     * Is deleted when this message is deleted.
     * @param contentSize Size of the content.
     */
    explicit CommandMessage(uint8_t receiver, bool group, bool groupAscending,
        uint8_t command, uint16_t messageID, uint8_t origin, uint8_t* content, uint16_t contentSize)
        : Message(receiver, group, groupAscending),
        command(command), messageID(messageID), origin(origin), content(content), contentSize(contentSize) {}

    /**
     * Constructor for dummy command messages.
     */
    explicit CommandMessage() : Message(0, 0),
        messageID(0), command(0), origin(0), content(nullptr), contentSize(0) {}

    /**
     * ID of the message.
     */
    uint16_t messageID;

    /**
     * Device that created the message.
     */
    uint8_t origin;

    /**
     * Command type of the message.
     */
    uint8_t command;

    /**
     * Parameters and other content of the message.
     */
    uint8_t* content;

    /**
     * Size of this message's content.
     */
    uint16_t contentSize;

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
 * Class for partial command messages. Used when command messages arrive, but consist of multiple packages.
 */
class PartialCommandMessage : public Message {
public:
    /**
     * Constructor for partial command messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param packageNumber Number of this package.
     * @param messageID ID of the message.
     * @param origin Device that created the message.
     * @param content Parameters and other content of the command.
     */
    explicit PartialCommandMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t packageNumber,
        uint16_t messageID, uint8_t origin, const uint8_t* content)
        : Message(receiver, typeAndGroups) {

        this->packageNumber = packageNumber;
        this->messageID = messageID;
        this->origin = origin;
        memcpy(this->content, content, COMMAND_SLOTS);
    }

    /**
     * Constructor for partial command messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param packageNumber Number of this package.
     * @param messageID ID of the message.
     * @param origin Device that created the message.
     * @param content Parameters and other content of the command.
     */
    explicit PartialCommandMessage(uint8_t receiver, bool group, bool groupAscending, uint8_t packageNumber,
        uint16_t messageID, uint8_t origin, const uint8_t* content)
        : Message(receiver, group, groupAscending) {

        this->packageNumber = packageNumber;
        this->messageID = messageID;
        this->origin = origin;
        memcpy(this->content, content, COMMAND_SLOTS);
    }

    /**
     * ID of the message.
     */
    uint16_t messageID;

    /**
     * Device that created the message.
     */
    uint8_t origin;

    /**
     * Package number of the message.
     */
    uint8_t packageNumber;

    /**
     * Parameters and other content of this part of the message.
     */
    uint8_t content[COMMAND_SLOTS];

    /**
     * Converts the messages to arrays of 32 bytes.
     * For messages larger than 32 bytes, the message is split and each message is added to the returned vector.
     * @return Vector with all messages.
     */
    std::vector<uint8_t*> getRawPackages() override {
        throw std::bad_function_call();
    };

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 0;
    }
};

/**
 * Class for register messages.
 */
class RegisterMessage : public Message {
public:
    using Message::Message;

    /**
     * Constructor for partial command messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param newDeviceID ID of the new device.
     */
    explicit RegisterMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t newDeviceID)
        : Message(receiver, typeAndGroups) {
        this->tempID = 0;
        this->newDeviceID = newDeviceID;
    }

    /**
     * Constructor for partial command messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param tempID Temporary ID for this device.
     */
    explicit RegisterMessage(uint8_t receiver, uint8_t typeAndGroups, uint32_t tempID)
        : Message(receiver, typeAndGroups) {
        this->tempID = tempID;
        this->newDeviceID = 0;
    }

    /**
     * Constructor for partial command messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param newDeviceID ID of the new device.
     */
    explicit RegisterMessage(uint8_t receiver, bool group, bool groupAscending, uint8_t newDeviceID)
        : Message(receiver, group, groupAscending) {
        this->tempID = 0;
        this->newDeviceID = newDeviceID;
    }

    /**
     * Constructor for partial command messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param tempID Temporary ID for this device.
     */
    explicit RegisterMessage(uint8_t receiver, bool group, bool groupAscending, uint32_t tempID)
        : Message(receiver, group, groupAscending) {
        this->tempID = tempID;
        this->newDeviceID = 0;
    }

    /**
     * Temporary ID if ID = 0.
     */
    uint32_t tempID;

    /**
     * ID of the new device.
     */
    uint8_t newDeviceID;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 1;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

/**
 * Class for message, that indicate whether a registration has benn accepted or rejected.
 */
class AcceptRejectMessage : public Message {
public:

    /**
     * Constructor for accept or reject messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param isAccept Indicates whether this is an accept or reject message.
     * @param tempID Temporary ID of the new device if receiver ID = 0.
     */
    explicit AcceptRejectMessage(uint8_t receiver, uint8_t typeAndGroups, bool isAccept, uint32_t tempID = 0)
        : Message(receiver, typeAndGroups) {
        this->isAccept = isAccept;
        this->tempID = tempID;
    }

    /**
     * Constructor for accept or reject messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param isAccept Indicates whether this is an accept or reject message.
     * @param tempID Temporary ID of the new device if receiver ID = 0.
     */
    explicit AcceptRejectMessage(uint8_t receiver, bool group, bool groupAscending, bool isAccept, uint32_t tempID = 0)
        : Message(receiver, group, groupAscending) {
        this->isAccept = isAccept;
        this->tempID = tempID;
    }

    /**
     * Temporary ID if ID = 0.
     */
    uint32_t tempID;

    /**
     * Indicates whether the new device is accepted or rejected.
     */
    bool isAccept;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 2;
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
     * @param typeAndGroups Message type and group flags.
     * @param pingId ID of this ping.
     * @param senderId Sender of this ping.
     * @param isResponse Indicates whether this is a ping or a response to one.
     * @param timestamp Timestamp of this message.
     */
    explicit PingMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t pingId, uint8_t senderId, bool isResponse, uint32_t timestamp)
        : Message(receiver, typeAndGroups) {
        this->pingId = pingId;
        this->senderId = senderId;
        this->isResponse = isResponse;
        this->timestamp = timestamp;
    }

    /**
    * Constructor for ping and ping and response messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param isResponse Indicates whether this is a ping or a response to one.
     * @param timestamp Timestamp of this message.
     */
    explicit PingMessage(uint8_t receiver, bool group, bool groupAscending, uint8_t pingId, uint8_t senderId, bool isResponse, uint32_t timestamp)
    : Message(receiver, group, groupAscending) {
        this->pingId = pingId;
        this->senderId = senderId;
        this->isResponse = isResponse;
        this->timestamp = timestamp;
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
     * Timestamp of this message to measure round trip time.
     */
    uint32_t timestamp;

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
 * Class for route creation messages.
 */
class RouteCreationMessage : public Message {
public:
    /**
     * Constructor for route creation messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param newDeviceID ID of the new device. 0 if the device has not been been registered before.
     */
    explicit RouteCreationMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t newDeviceID)
    : Message(receiver, typeAndGroups) {
        this->newDeviceID = newDeviceID;
        this->tempID = 0;
    }

    /**
     * Constructor for route creation messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param newDeviceID ID of the new device. 0 if the device has not been been registered before.
     */
    explicit RouteCreationMessage(uint8_t receiver, bool group, bool groupAscending, uint8_t newDeviceID)
    : Message(receiver, group, groupAscending) {
        this->newDeviceID = newDeviceID;
        this->tempID = 0;
    }

    /**
     * Constructor for route creation messages.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param tempID Temporary ID if ID = 0.
     */
    explicit RouteCreationMessage(uint8_t receiver, uint8_t typeAndGroups, uint32_t tempID)
        : Message(receiver, typeAndGroups) {
        this->newDeviceID = 0;
        this->tempID = tempID;
    }

    /**
     * Constructor for route creation messages.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param tempID Temporary ID if ID = 0.
     */
    explicit RouteCreationMessage(uint8_t receiver, bool group, bool groupAscending, uint32_t tempID)
        : Message(receiver, group, groupAscending) {
        this->newDeviceID = 0;
        this->tempID = tempID;
    }

    /**
     * Temporary ID if ID = 0.
     */
    uint32_t tempID;

    /**
     * ID of the new device. 0 if the device has not been been registered before.
     */
    uint8_t newDeviceID;

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
 * Class for messages to add or remove devices to/from a group.
 */
class AddRemoveToGroupMessage : public Message {
public:
    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param groupId ID of the group.
     * @param isAddToGroup True if the device has to be added to the given group, false if it has to be removed from it.
     */
    explicit AddRemoveToGroupMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t groupId, bool isAddToGroup)
        : Message(receiver, typeAndGroups) {
        this->groupId = groupId;
        this->isAddToGroup = isAddToGroup;
    }

    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param groupId ID of the group.
     * @param isAddToGroup True if the device has to be added to the given group, false if it has to be removed from it.
     */
    explicit AddRemoveToGroupMessage(uint8_t receiver, bool group, bool groupAscending, uint8_t groupId, bool isAddToGroup)
        : Message(receiver, group, groupAscending) {
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
        return 5;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

/**
 * Class for error messages.
 */
class ErrorMessage : public Message {
public:
    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param errorCode Code of the occurred error.
     * @param erroneousMessage The message that cause the error.
     */
    explicit ErrorMessage(uint8_t receiver, uint8_t typeAndGroups, uint8_t errorCode, const uint8_t* erroneousMessage)
        : Message(receiver, typeAndGroups) {
        this->errorCode = errorCode;
        this->erroneousMessage = erroneousMessage;
    }

    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param errorCode Code of the occurred error.
     * @param erroneousMessage The message that cause the error.
     */
    explicit ErrorMessage(uint8_t receiver, bool group, bool groupAscending, uint8_t errorCode, const uint8_t* erroneousMessage)
        : Message(receiver, group, groupAscending) {
        this->errorCode = errorCode;
        this->erroneousMessage = erroneousMessage;
    }

    /**
     * Code of the occurred error.
     */
    uint8_t errorCode;

    /**
     * First up to 28 bytes of the message that cause the error.
     */
    const uint8_t *erroneousMessage;

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

/**
 * Class for discovery messages to find near devices.
 */
class DiscoverMessage : public Message {
public:
    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param isRequest True if this message is a request for discovery, False if it is a response.
     * @param ID ID of the discovering device.
     */
    explicit DiscoverMessage(uint8_t receiver, uint8_t typeAndGroups, bool isRequest, uint8_t ID)
        : Message(receiver, typeAndGroups) {
        this->isRequest = isRequest;
        this->tempID = 0;
        this->ID = ID;
    }

    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param isRequest True if this message is a request for discovery, False if it is a response.
     * @param ID ID of the discovering device.
     */
    explicit DiscoverMessage(uint8_t receiver, bool group, bool groupAscending, bool isRequest, uint8_t ID)
        : Message(receiver, group, groupAscending) {
        this->isRequest = isRequest;
        this->tempID = 0;
        this->ID = ID;
    }

    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param isRequest True if this message is a request for discovery, False if it is a response.
     * @param tempID Temporary ID if ID = 0.
     */
    explicit DiscoverMessage(uint8_t receiver, uint8_t typeAndGroups, bool isRequest, uint32_t tempID)
        : Message(receiver, typeAndGroups) {
        this->isRequest = isRequest;
        this->tempID = tempID;
        this->ID = 0;
    }

    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param isRequest True if this message is a request for discovery, False if it is a response.
     * @param tempID Temporary ID if ID = 0.
     */
    explicit DiscoverMessage(uint8_t receiver, bool group, bool groupAscending, bool isRequest, uint32_t tempID)
        : Message(receiver, group, groupAscending) {
        this->isRequest = isRequest;
        this->tempID = tempID;
        this->ID = 0;
    }

    /**
     * Temporary ID if ID = 0.
     */
    uint32_t tempID;

    /**
     * ID of the discovering device.
     */
    uint8_t ID;

    /**
     * True if this message is a request for discovery, False if it is a response.
     */
    bool isRequest;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 7;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

/**
 * Class for reconnect and disconnect messages.
 */
class ReDisconnectMessage : public Message {
public:
    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param typeAndGroups Message type and group flags.
     * @param isDisconnect True if this message is a disconnect message, False if it is a reconnect message.
     */
    explicit ReDisconnectMessage(uint8_t receiver, uint8_t typeAndGroups, bool isDisconnect)
        : Message(receiver, typeAndGroups) {
        this->isDisconnect = isDisconnect;
    }

    /**
     * Constructor for messages to add or remove devices to/from a group.
     * @param receiver Receiver of this message.
     * @param group Is the receiver a group.
     * @param groupAscending Is the receiver ascending to the hub.
     * @param isDisconnect True if this message is a disconnect message, False if it is a reconnect message.
     */
    explicit ReDisconnectMessage(uint8_t receiver, bool group, bool groupAscending, bool isDisconnect)
        : Message(receiver, group, groupAscending) {
        this->isDisconnect = isDisconnect;
    }

    /**
     * True if this message is a disconnect message, False if it is a reconnect message.
     */
    bool isDisconnect;

    /**
     * @return Type of this message.
     */
    uint8_t getType() override {
        return 8;
    }

    /**
     * Creates a vector with only one element, which is a byte representation of this message.
     * @return A vector with byte representation of this message.
     */
    std::vector<uint8_t*> getRawPackages() override;
};

#endif //NETWORKPROTOCOL_MESSAGEOBJECTS_H