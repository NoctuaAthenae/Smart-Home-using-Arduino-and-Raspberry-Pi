#include "messageObjects.h"
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos,set) ((var) | (set<<(pos)))

bool Message::isGroup() {
    return CHECK_BIT(this->typeAndGroups, 0);
}

bool Message::isGroupAscending() {
    return CHECK_BIT(this->typeAndGroups, 1);
}

void Message::setGroup(bool set) {
    this->typeAndGroups = SET_BIT(this->typeAndGroups, 0, set);
}

void Message::setGroupAscending(bool set) {
    this->typeAndGroups = SET_BIT(this->typeAndGroups, 1, set);
}

void Message::addChecksum() {
    // TODO implement checksum
}

std::vector<uint8_t*> Message::getRawPackages() {
    uint8_t rawPackage[32] = {
        this->version,
        this->receiver,
        this->lastDeviceId,
        this->nextHop,
        this->typeAndGroups,
        this->checksum,
    };

    std::memcpy(rawPackage + 6, &(this->timestamp), 4);

    return {rawPackage};
}

std::vector<uint8_t*> CommandMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    uint8_t* templatePackage = rawPackages[0];

    // TODO create package array

    return rawPackages;
}

std::vector<uint8_t*> AcceptRejectMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->givenId;
    rawPackages.at(0)[11] = this->isAccept;
    return rawPackages;
}

std::vector<uint8_t*> PingMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->pingId;
    rawPackages.at(0)[11] = this->senderId;
    rawPackages.at(0)[12] = this->isResponse;
    return rawPackages;
}

std::vector<uint8_t*> RouteCreationMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->newId;
    for (int i = 0; i < 21; ++i)
        rawPackages.at(0)[11 + i] = this->route[i];
    return rawPackages;
}

std::vector<uint8_t*> AddRemoveToGroupMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->groupId;
    rawPackages.at(0)[11] = this->isAddToGroup;
    return rawPackages;
}
