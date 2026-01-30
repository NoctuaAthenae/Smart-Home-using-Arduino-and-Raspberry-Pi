#include "messageObjects.h"

#include <cstdlib>
#include <cstring>
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos,set) ((var) | (set<<(pos)))

uint8_t Message::getGroupTypeByte() {
    uint8_t byte = this->getType() * 2;
    byte = SET_BIT(byte, 0, this->group);
    return byte;
}

Message *Message::fromRawBytes(const uint8_t *rawPackage) {
    uint8_t type = rawPackage[2] >> 1;

    switch (type) {
        case 0: {
            uint16_t id = 0;
            memcpy(&id, rawPackage + 5, 2);
            return new PartialDataMessage(rawPackage[1], CHECK_BIT(rawPackage[2], 0), rawPackage[3], id,
                rawPackage[4], rawPackage + METADATA_SLOTS);
        }
        case 1: {
            uint8_t newDeviceId = rawPackage[4];
            uint32_t id = 0;
            memcpy(&id, rawPackage + 5, 4);
            return new RegistrationMessage(rawPackage[1], newDeviceId, id, rawPackage[3], static_cast<bool>(rawPackage[9]));
        }
        case 2: {
            uint32_t timestamp = 0;
            memcpy(&timestamp, rawPackage + 6, 4);
            return new PingMessage(rawPackage[1], rawPackage[4], rawPackage[3], rawPackage[5], timestamp);
        }
        case 3: {
            return new AddRemoveToGroupMessage(rawPackage[1], rawPackage[4], rawPackage[3]);
        }
        case 4: {
            return new ErrorMessage(rawPackage[1], rawPackage[3], rawPackage + 4);
        }
        case 5: {
            return new ReDisconnectMessage(rawPackage[1], rawPackage[3]);
        }
        default: {
            break;
        }
    }
    return nullptr;
}

uint8_t Message::getRawPackages(uint8_t** data) {
    auto *rawPackage = new uint8_t[32] {
        this->version,
        this->receiver,
        this->getGroupTypeByte()
    };

    *data = rawPackage;

    return 1;
}

void Message::cleanUp(const uint8_t* packages) {
    delete[] packages;
}


uint8_t DataMessage::getRawPackages(uint8_t** data) {

    // first package has 24 slots, all others 25
    // if 25 slots are used: 25 / 25 + 1 = 1 + 1 = 2
    uint8_t numberPackages = (this->contentSize + FIRST_METADATA_SLOTS - 1) / DATA_SLOTS + 1;

    // set up the metadata of the message using the base function
    // use the created package as a template
    uint8_t *templatePackageAddress[1];
    Message::getRawPackages(templatePackageAddress);
    uint8_t* templatePackage = *templatePackageAddress;

    // set meta data that does not differ between the packages
    templatePackage[4] = this->origin;
    memcpy(templatePackage + 5, &this->messageID, 2);

    // raw packages for the data returned
    auto* rawPackages = new uint8_t[numberPackages * 32];
    *data = rawPackages;

    // the last package is not completely filled
    // remove 24 for first package and 25 for all other packages beside the last
    uint8_t lastPackageSize = this->contentSize -
        (numberPackages == 1 ? 0 : SLOT_COUNT(numberPackages - 1) - 2);

    for (uint8_t i = 0; i < numberPackages; i++) {
        // create a new raw package, copy the metadata from the template and set the package number
        auto *package = rawPackages + i * 32;
        memcpy(package, templatePackage, METADATA_SLOTS);
        package[3] = i;

        uint8_t extraMetaDataSize = 0;
        uint8_t startingIndex = SLOT_COUNT(i);

        // first package saves total number of packages
        if (i == 0) {
            rawPackages[7] = numberPackages;
            extraMetaDataSize = FIRST_METADATA_SLOTS;
            startingIndex = 0;
        }

        // copy the corresponding content into the slots
        memcpy(package + METADATA_SLOTS + extraMetaDataSize, this->content + startingIndex,
            i == numberPackages - 1 ? lastPackageSize : DATA_SLOTS - extraMetaDataSize);
    }

    cleanUp(templatePackage);

    return numberPackages;
}

uint8_t RegistrationMessage::getRawPackages(uint8_t** data) {
    Message::getRawPackages(data);
    (*data)[3] = this->registrationType;
    (*data)[4] = this->newDeviceID;
    memcpy((*data) + 5, &this->tempID, 4);
    (*data)[9] = this->extraField;
    return 1;
}

uint8_t PingMessage::getRawPackages(uint8_t** data) {
    Message::getRawPackages(data);
    (*data)[3] = this->senderId;
    (*data)[4] = this->pingId;
    (*data)[5] = this->isResponse;
    memcpy((*data) + 6, &this->timestamp, 4);
    return 1;
}

uint8_t AddRemoveToGroupMessage::getRawPackages(uint8_t** data) {
    Message::getRawPackages(data);
    (*data)[3] = this->isAddToGroup;
    (*data)[4] = this->groupId;
    return 1;
}

uint8_t ErrorMessage::getRawPackages(uint8_t** data) {
    Message::getRawPackages(data);
    (*data)[3] = this->errorCode;
    memcpy((*data) + 4, this->erroneousMessage, 28);
    return 1;
}

uint8_t ReDisconnectMessage::getRawPackages(uint8_t** data) {
    Message::getRawPackages(data);
    (*data)[3] = this->isDisconnect;
    return 1;
}
