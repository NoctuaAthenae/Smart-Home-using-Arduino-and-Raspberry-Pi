
#include "messageBuilder.h"

#include <iostream>
#include <ostream>

#include "messageObjects.h"

bool MessageBuilder::newDataMessage(PartialDataMessage *message, DataMessage *result) {

    // messages are identified by origin and message ID
    std::pair<uint8_t, uint16_t> key = std::pair<uint8_t, uint16_t>(message->origin, message->messageID);

    // if no package of the message has been received yet, create a new entry
    if (this->partialDatas.find(key) == this->partialDatas.end()) {
        this->partialDatas.insert(std::pair<std::pair<uint8_t, uint16_t>,
            std::vector<PartialDataMessage*>>(key, std::vector<PartialDataMessage*>()));
    }

    std::vector<PartialDataMessage*>* partialDatas = &this->partialDatas.at(key);

    // add the message to the list of partial messages
    partialDatas->push_back(message);

    if (message->packageNumber == 0) {
        // first package has information about the number of packages
        this->numPackages.insert(std::pair<std::pair<uint8_t, uint16_t>, uint8_t>(key, message->content[0]));
    } else if (this->numPackages.find(key) == this->numPackages.end()) {
        // first package has not arrived yet
        return false;
    }

    // check if all packages have arrived
    if (partialDatas->size() != this->numPackages.at(key)) {
        return false;
    }

    uint8_t size = SLOT_COUNT(this->numPackages.at(key));

    result->receiver = message->receiver;
    result->group = message->group;
    result->messageID = message->messageID;
    result->origin = message->origin;
    result->content = new uint8_t[size];
    result->contentSize = size;

    // assemble the content of the message
    // iterate through packages to find the next package in order
    for (uint8_t i = 0; i < partialDatas->size(); i++) {
        PartialDataMessage *partialData = partialDatas->at(i);

        if (partialDatas->at(i)->packageNumber == 0) {
            // first package saves data and has less content
            memcpy(result->content, partialData->content + FIRST_METADATA_SLOTS, FIRST_DATA_PACKAGE_SLOTS);
        } else {
            memcpy(result->content + SLOT_COUNT(partialDatas->at(i)->packageNumber), partialData->content, DATA_SLOTS);
        }

        delete partialData;
    }

    this->partialDatas.erase(key);
    this->numPackages.erase(key);

    return true;
}

