#ifndef NETWORKPROTOCOL_MESSAGEOBJECTS_H
#define NETWORKPROTOCOL_MESSAGEOBJECTS_H
#include <cstdint>

class Message {
public:
    virtual ~Message() = default;

    uint32_t timestamp;
    uint8_t version;
    uint8_t receiver;
    uint8_t lastDeviceId;
    uint8_t nextHop;
    uint8_t typeAndGroups;
    uint8_t checksum;

    virtual uint8_t getType();

    bool isGroup();
    void setGroup(bool);
    bool isGroupAscending();
    void setGroupAscending(bool);
    void addChecksum();

    Message(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups) {
        this->version = 0;
        this->receiver = receiver;
        this->lastDeviceId = lastDeviceId;
        this->nextHop = nextHop;
        this->typeAndGroups = typeAndGroups;
        this->timestamp = 0;
        this->checksum = 0;
    }
};

class RegisterMessage : public Message {
public:
    using Message::Message;

    uint8_t getType() override {
        return 2;
    }
};

class AcceptRejectMessage : public Message {
public:
    explicit AcceptRejectMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t givenId, bool isAccept)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->givenId = givenId;
        this->isAccept = isAccept;
    }

    uint8_t givenId;
    bool isAccept;

    uint8_t getType() override {
        return 3;
    }
};

class RouteCreationMessage : public Message {
public:
    explicit RouteCreationMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t newId, uint8_t route*, uint8_t numberHopsInRoute)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->newId = newId;
        for (int i = 0; i < numberHopsInRoute; i++) {
            this->route[i] = route[i];
        }
    }

    uint8_t newId;
    uint8_t route[21];

    uint8_t getType() override {
        return 5;
    }
};

class PingMessage : public Message {
public:
    explicit PingMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t pingId, uint8_t senderId, bool isResponse)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->pingId = pingId;
        this->senderId = senderId;
        this->isResponse = isResponse;
    }

    uint8_t pingId;
    uint8_t senderId;
    bool isResponse;

    uint8_t getType() override {
        return 4;
    }
};

class AddRemoveToGroupMessage : public Message {
public:
    explicit AddRemoveToGroupMessage(uint8_t receiver, uint8_t lastDeviceId, uint8_t nextHop, uint8_t typeAndGroups, uint8_t groupId, bool isAddToGroup)
        : Message(receiver, lastDeviceId, nextHop, typeAndGroups) {
        this->groupId = groupId;
        this->isAddToGroup = isAddToGroup;
    }

    bool isAddToGroup;
    uint8_t groupId;

    uint8_t getType() override {
        return 6;
    }
};

#endif //NETWORKPROTOCOL_MESSAGEOBJECTS_H