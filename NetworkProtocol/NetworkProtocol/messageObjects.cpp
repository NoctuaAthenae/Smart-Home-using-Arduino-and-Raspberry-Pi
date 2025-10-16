#include "messageObjects.h"

#include <cstdlib>
#include <cstring>
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

Message *Message::fromRawBytes(const uint8_t *rawPackage) {
    uint8_t type = rawPackage[2] >> 2;

    switch (type) {
        case 0: {
            uint16_t id = 0;
            memcpy(&id, rawPackage + 5, 2);
            return new PartialCommandMessage(rawPackage[1], rawPackage[2], rawPackage[3], id,
                rawPackage[4], rawPackage + COMMAND_METADATA_SLOTS);
        }
        case 1: {
            uint8_t newDeviceId = rawPackage[3];
            if (newDeviceId == 0) {
                uint32_t id = 0;
                memcpy(&id, rawPackage + 4, 4);
                return new RegisterMessage(rawPackage[1], rawPackage[2], id);
            }
            return new RegisterMessage(rawPackage[1], rawPackage[2], newDeviceId);
        }
        case 2: {
            uint8_t receiver = rawPackage[1];
            uint32_t id = 0;
            if (receiver == 0) {
                memcpy(&id, rawPackage + 4, 4);
            }
            return new AcceptRejectMessage(receiver, rawPackage[2], rawPackage[3], id);
        }
        case 3: {
            uint32_t timestamp = 0;
            memcpy(&timestamp, rawPackage + 6, 4);
            return new PingMessage(rawPackage[1], rawPackage[2], rawPackage[4], rawPackage[3], rawPackage[5], timestamp);
        }
        case 4: {
            uint8_t newDeviceId = rawPackage[3];
            if (newDeviceId == 0) {
                uint32_t id = 0;
                memcpy(&id, rawPackage + 4, 4);
                return new RouteCreationMessage(rawPackage[1], rawPackage[2], id);
            }
            return new RouteCreationMessage(rawPackage[1], rawPackage[2], newDeviceId);
        }
        case 5: {
            return new AddRemoveToGroupMessage(rawPackage[1], rawPackage[2], rawPackage[4], rawPackage[3]);
        }
        case 6: {
            return new ErrorMessage(rawPackage[1], rawPackage[2], rawPackage[3], rawPackage + 4);
        }
        case 7: {
            uint8_t deviceId = rawPackage[4];
            uint32_t tempId = 0;
            if (deviceId == 0) {
                memcpy(&tempId, rawPackage + 5, 4);
                return new DiscoverMessage(rawPackage[1], rawPackage[2], rawPackage[3], rawPackage[4], tempId);
            }
            return new DiscoverMessage(rawPackage[1], rawPackage[2], rawPackage[3], rawPackage[4], deviceId);
        }
        case 8: {
            return new ReDisconnectMessage(rawPackage[1], rawPackage[2], rawPackage[3]);
        }
        default: {
            break;
        }
    }
    return nullptr;
}



std::vector<uint8_t*> Message::getRawPackages() {
    auto *rawPackage = new uint8_t[32] {
        this->version,
        this->receiver,
        this->typeAndGroups
    };

    // if smaller than 4, type is not set yet
    if (this->typeAndGroups < 4) rawPackage[2] += this->getType() << 2;

    return {rawPackage};
}

void Message::cleanUp(const std::vector<uint8_t *>* packages) {
    for (auto & package : *packages) {
        delete[] package;
    }
}


std::vector<uint8_t*> CommandMessage::getRawPackages() {

    // set up the metadata of the message using the base function
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();

    // first package has 23 slots, all others 25
    // if 24 slots are used: (24 + 1) / 25 + 1 = 25 / 25 + 1 = 2
    uint8_t numberPackages = (this->content.size() + 1) / COMMAND_SLOTS + 1;

    // use the created package as a template
    uint8_t* templatePackage = rawPackages.at(0);

    // the last package is not completely filled
    // remove 23 for first package and 25 for all other packages beside the last
    uint8_t lastPackageSize = this->content.size() -
        (numberPackages == 1 ? 0 : SLOT_COUNT(numberPackages - 1) - 2);

    for (uint8_t i = 0; i < numberPackages; i++) {
        // first package saves total number of packages number of package and command
        if (i == 0) {
            rawPackages.at(0)[3] = i;
            rawPackages.at(0)[4] = this->origin;
            memcpy(rawPackages.at(0) + 5, &this->messageID, 2);
            rawPackages.at(0)[7] = this->command;
            rawPackages.at(0)[8] = numberPackages;
            memcpy(rawPackages.at(0) + 9, &this->content.at(0),
                i == numberPackages - 1 ? lastPackageSize : (COMMAND_SLOTS - 2));
            continue;
        }

        // create a new raw package, copy the metadata from the template and set the package number
        auto *package = new uint8_t[32];
        memcpy(package, templatePackage, COMMAND_METADATA_SLOTS);
        package[3] = i;
        package[4] = this->origin;
        memcpy(package + 5, &this->messageID, 2);

        // copy the corresponding content into the slots
        memcpy(package + COMMAND_METADATA_SLOTS, &this->content.at(SLOT_COUNT(i)),
            i == numberPackages - 1 ? lastPackageSize : COMMAND_SLOTS);

        rawPackages.push_back(package);
    }

    return rawPackages;
}

std::vector<uint8_t*> RegisterMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->newDeviceID;
    memcpy(rawPackages.at(0) + 4, &this->tempID, 4);
    return rawPackages;
}

std::vector<uint8_t*> AcceptRejectMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->isAccept;
    memcpy(rawPackages.at(0) + 4, &this->tempID, 4);
    return rawPackages;
}

std::vector<uint8_t*> PingMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->senderId;
    rawPackages.at(0)[4] = this->pingId;
    rawPackages.at(0)[5] = this->isResponse;
    memcpy(rawPackages.at(0) + 6, &this->timestamp, 4);
    return rawPackages;
}

std::vector<uint8_t*> RouteCreationMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->newDeviceID;
    memcpy(rawPackages.at(0) + 4, &this->tempID, 4);
    return rawPackages;
}

std::vector<uint8_t*> AddRemoveToGroupMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->isAddToGroup;
    rawPackages.at(0)[4] = this->groupId;
    return rawPackages;
}

std::vector<uint8_t*> ErrorMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->errorCode;
    memcpy(rawPackages.at(0) + 4, this->erroneousMessage, 28);
    return rawPackages;
}

std::vector<uint8_t*> DiscoverMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->isRequest;
    rawPackages.at(0)[4] = this->ID;
    memcpy(rawPackages.at(0) + 5, &this->tempID, 4);
    return rawPackages;
}

std::vector<uint8_t*> ReDisconnectMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[3] = this->isDisconnect;
    return rawPackages;
}
