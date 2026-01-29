
#include "messageBuilder.h"

#include <iostream>
#include <ostream>

#include "messageObjects.h"

bool MessageBuilder::newCommandMessage(PartialCommandMessage *message, CommandMessage *result) {

    // messages are identified by origin and message ID
    std::pair<uint8_t, uint16_t> key = std::pair<uint8_t, uint16_t>(message->origin, message->messageID);

    // if no package of the message has been received yet, create a new entry
    if (this->partialCommands.find(key) == this->partialCommands.end()) {
        this->partialCommands.insert(std::pair<std::pair<uint8_t, uint16_t>,
            std::vector<PartialCommandMessage*>>(key, std::vector<PartialCommandMessage*>()));
    }

    std::vector<PartialCommandMessage*>* partialCommands = &this->partialCommands.at(key);

    // add the message to the list of partial messages
    partialCommands->push_back(message);

    if (message->packageNumber == 0) {
        // first package has information about the number of packages
        this->numPackages.insert(std::pair<std::pair<uint8_t, uint16_t>, uint8_t>(key, message->content[1]));
    } else if (this->numPackages.find(key) == this->numPackages.end()) {
        // first package has not arrived yet
        return false;
    }

    // check if all packages have arrived
    if (partialCommands->size() != this->numPackages.at(key)) {
        return false;
    }

    uint8_t size = SLOT_COUNT(this->numPackages.at(key));

    result->receiver = message->receiver;
    result->typeAndGroups = message->typeAndGroups;
    result->messageID = message->messageID;
    result->origin = message->origin;
    result->content = new uint8_t[size];
    result->contentSize = size;

    // assemble the content of the message
    // iterate through packages to find the next package in order
    for (uint8_t i = 0; i < partialCommands->size(); i++) {
        PartialCommandMessage *partialCommand = partialCommands->at(i);

        if (partialCommands->at(i)->packageNumber == 0) {
            // first package saves command and has less content
            memcpy(result->content, partialCommand->content + 2, FIRST_COMMAND_SLOTS);
            result->command = partialCommand->content[0];
        } else {
            memcpy(result->content + SLOT_COUNT(partialCommands->at(i)->packageNumber), partialCommand->content, COMMAND_SLOTS);
        }

        delete partialCommand;
    }

    this->partialCommands.erase(key);
    this->numPackages.erase(key);

    return true;
}

