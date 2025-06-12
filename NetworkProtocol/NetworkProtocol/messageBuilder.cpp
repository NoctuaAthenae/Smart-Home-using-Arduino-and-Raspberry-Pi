
#include "messageBuilder.h"

bool MessageBuilder::newCommandMessage(PartialCommandMessage *message, CommandMessage *result) {

    //TODO identification over other fields

    // messages are identified by lastDeviceId and timestamp
    std::pair<uint8_t, uint32_t> key = std::pair<uint8_t, uint32_t>(message->lastDeviceId, message->timestamp);

    // if no package of the message has been received yet, create a new entry
    if (partialCommands.find(key) == partialCommands.end()) {
        partialCommands.insert(std::pair<std::pair<uint8_t, uint32_t>, std::vector<PartialCommandMessage*>>(key, std::vector<PartialCommandMessage*>()));
    }

    std::vector<PartialCommandMessage*>* partialCommands = &this->partialCommands.at(key);

    // add the message to the list of partial messages
    partialCommands->push_back(message);

    if (message->packageNumber == 0) {
        // first package has information about the number of packages
        numPackages.insert(std::pair<std::pair<uint8_t, uint32_t>, uint8_t>(key, (*message->content)[0]));
    } else if (numPackages.find(key) == numPackages.end()) {
        // first package has not arrived yet
        return false;
    }

    // check if all packages have arrived
    if (partialCommands->size() != numPackages.at(key)) {
        return false;
    }

    *result = CommandMessage(message->receiver, message->lastDeviceId, message->nextHop, message->isGroup(), message->isGroupAscending(), 0, new std::vector<uint8_t>());

    // assemble the content of the message
    // iterate through packages to find the next package in order
    for (uint8_t i = 0; i < numPackages.at(key); i++) {
        for (uint8_t j = 0; j < numPackages.at(key); j++) {
            if (partialCommands->at(j)->packageNumber == i) {
                PartialCommandMessage partialCommand = *partialCommands->at(j);

                if (i == 0) {
                    // first package saves command and has less content
                    result->content->insert(result->content->end(), partialCommand.content->begin() + 2, partialCommand.content->end());
                    result->command = (*partialCommand.content)[1];
                    break;
                }

                result->content->insert(result->content->end(), partialCommand.content->begin(), partialCommand.content->end());
                break;
            }
        }
    }

    result->timestamp = message->timestamp;

    return true;
}

