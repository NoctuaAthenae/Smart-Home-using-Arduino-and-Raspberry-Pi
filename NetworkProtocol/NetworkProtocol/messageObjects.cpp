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
    for (auto byte: rawPackage) {
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



std::vector<uint8_t*> Message::getRawPackages() {
    auto *rawPackage = new uint8_t[32] {
        this->version,
        this->receiver,
        this->lastDeviceId,
        this->nextHop,
        this->typeAndGroups,
        0
    };

    std::memcpy(rawPackage + 6, &(this->timestamp), 4);

    return {rawPackage};
}

void Message::cleanUp(const std::vector<uint8_t *>* packages) {
    for (auto & package : *packages) {
        delete[] package;
    }
}


std::vector<uint8_t*> CommandMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();

    uint8_t numberPackages = this->content.size() / 20 + 1;

    rawPackages.at(0)[11] = numberPackages;
    uint8_t* templatePackage = rawPackages[0];

    uint8_t lastPackageSize = this->content.size() -
        (numberPackages == 1 ? 0 : (19 + (numberPackages - 1) * 20));

    for (uint8_t i = 0; i < numberPackages; i++) {
        if (i == 0) {
            rawPackages.at(0)[10] = 0;
            rawPackages.at(0)[12] = this->command;
            continue;
        }

        auto *package = new uint8_t[32];
        memcpy(package, templatePackage, 32);
        package[10] = i;

        memcpy(package + 12, &this->content[19 + 20 * (i - 1)], i == numberPackages - 1 ? lastPackageSize : 20);

        addChecksum(package);
        rawPackages.push_back(package);
    }

    return rawPackages;
}

std::vector<uint8_t*> AcceptRejectMessage::getRawPackages() {
    std::vector<uint8_t*> rawPackages = Message::getRawPackages();
    rawPackages.at(0)[10] = this->givenId;
    rawPackages.at(0)[11] = this->isAccept;
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
