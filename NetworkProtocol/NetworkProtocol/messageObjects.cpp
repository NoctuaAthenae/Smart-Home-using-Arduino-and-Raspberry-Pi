#include "messageObjects.h"

#include <cstdlib>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos,set) ((var) | (set<<(pos)))
#define CRC_POLY 0xe7

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

bool Message::checkChecksum(uint8_t *rawPackage) {
    uint8_t givenCrc = rawPackage[5];
    rawPackage[5] = 0;
    return givenCrc == getChecksum(rawPackage);
}

void Message::addChecksum(uint8_t* rawPackage) {
    rawPackage[5] = getChecksum(rawPackage);
}

uint8_t Message::getChecksum(uint8_t *rawPackage) {
    uint8_t crc = 255;
    for (uint8_t index = 0; index < 32; ++index) {
        uint8_t byte = rawPackage[index];
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 128) {
                crc = (crc * 2 + (byte & 128 ? 1 : 0) ) ^ CRC_POLY;
            } else {
                crc = crc * 2 + (byte & 128 ? 1 : 0);
            }
            byte <<= 1;
        }
    }
    return crc;
}

Message *Message::fromRawBytes(const uint8_t *rawPackage) {
    uint8_t type = rawPackage[4] >> 2;

    switch (type) {
        case 0: {
            return new PartialCommandMessage(rawPackage[1], rawPackage[2], rawPackage[3], rawPackage[4],
                rawPackage[10], rawPackage + 11, rawPackage + 6);
        }
        case 1: {
            return new AcknowledgeMessage(rawPackage[1], rawPackage[2], rawPackage[4],
                rawPackage + 6);
        }
        case 2: {
            return new RegisterMessage(rawPackage[1], rawPackage[2], rawPackage[3],
                rawPackage[4]);
        }
        case 3: {
            return new AcceptRejectMessage(rawPackage[1], rawPackage[2], rawPackage[3],
                rawPackage[4], rawPackage[10]);
        }
        case 4: {
            return new PingMessage(rawPackage[1], rawPackage[2], rawPackage[3],
                rawPackage[4], rawPackage[10], rawPackage[11], rawPackage[12]);
        }
        case 5: {
            uint8_t numberHopsInRoute = 0;
            while (rawPackage[11 + numberHopsInRoute] != 0) numberHopsInRoute++;
            return new RouteCreationMessage(rawPackage[1], rawPackage[2], rawPackage[3],
                rawPackage[4], rawPackage[10], rawPackage + 11, numberHopsInRoute);
        }
        case 6: {
            return new AddRemoveToGroupMessage(rawPackage[1], rawPackage[2], rawPackage[3],
                rawPackage[4], rawPackage[10], rawPackage[11]);
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
        this->lastDeviceId,
        this->nextHop,
        this->typeAndGroups,
        0
    };

    rawPackage[4] += this->getType() << 2;

    std::memcpy(rawPackage + 6, &(this->timestamp), 4);

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

    // first package has 19 slots, all others 21
    // if 20 slots are used: (20 + 1) / 21 + 1 = 21 / 21 + 1 = 2
    uint8_t numberPackages = (this->content->size() + 1) / 21 + 1;

    // use the created package as a template
    uint8_t* templatePackage = rawPackages.at(0);

    // the last package is not completely filled
    // remove 19 for first package and 21 for all other packages beside the last
    uint8_t lastPackageSize = this->content->size() -
        (numberPackages == 1 ? 0 : (19 + (numberPackages - 2) * 21));

    for (uint8_t i = 0; i < numberPackages; i++) {
        // first package saves total number of packages number of package and command
        if (i == 0) {
            rawPackages.at(0)[10] = 0;
            rawPackages.at(0)[11] = numberPackages;
            rawPackages.at(0)[12] = this->command;
            memcpy(rawPackages.at(0) + 13, &this->content->at(0), i == numberPackages - 1 ? lastPackageSize : 19);
            addChecksum(rawPackages.at(0));
            continue;
        }

        // create a new raw package, copy the metadata from the template and set the package number
        auto *package = new uint8_t[32];
        memcpy(package, templatePackage, 10);
        package[10] = i;

        // copy the corresponding content into the slots
        memcpy(package + 11, &this->content->at(19 + 21 * (i - 1)), i == numberPackages - 1 ? lastPackageSize : 21);

        addChecksum(package);
        rawPackages.push_back(package);
    }

    return rawPackages;
}

std::vector<uint8_t*> AcknowledgeMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    addChecksum(rawPackages.at(0));
    return rawPackages;
}

std::vector<uint8_t*> RegisterMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    addChecksum(rawPackages.at(0));
    return rawPackages;
}

std::vector<uint8_t*> AcceptRejectMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->isAccept;
    addChecksum(rawPackages.at(0));
    return rawPackages;
}

std::vector<uint8_t*> PingMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->pingId;
    rawPackages.at(0)[11] = this->senderId;
    rawPackages.at(0)[12] = this->isResponse;
    addChecksum(rawPackages.at(0));
    return rawPackages;
}

std::vector<uint8_t*> RouteCreationMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->newId;
    int i = 0;
    while (i < 21 && this->route[i] != 0) {
        rawPackages.at(0)[11 + i] = this->route[i];
        ++i;
    }
    rawPackages.at(0)[11 + i] = this->lastDeviceId;
    addChecksum(rawPackages.at(0));
    return rawPackages;
}

std::vector<uint8_t*> AddRemoveToGroupMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->groupId;
    rawPackages.at(0)[11] = this->isAddToGroup;
    addChecksum(rawPackages.at(0));
    return rawPackages;
}
