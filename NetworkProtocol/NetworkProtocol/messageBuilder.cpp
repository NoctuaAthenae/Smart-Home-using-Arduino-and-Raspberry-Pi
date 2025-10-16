
#include "messageBuilder.h"

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

    uint8_t size = (this->numPackages.at(key) - 1) * COMMAND_SLOTS + FIRST_COMMAND_SLOTS;

    *result = CommandMessage(message->receiver, message->isGroup(), message->isGroupAscending(), 0, message->messageID, message->origin, new std::vector<uint8_t>(size));

    // assemble the content of the message
    // iterate through packages to find the next package in order
    for (uint8_t i = 0; i < this->numPackages.at(key); i++) {
        PartialCommandMessage partialCommand = *partialCommands->at(i);

        if (partialCommands->at(i)->packageNumber == 0) {
            // first package saves command and has less content
            memcpy(&result->content->at(0), partialCommand.content + 2, FIRST_COMMAND_SLOTS);
            //result->content->insert(result->content->end(), partialCommand.content + 2, partialCommand.content + FIRST_COMMAND_SLOTS);
            result->command = partialCommand.content[0];
        }

        memcpy(&result->content->at(0) + SLOT_COUNT(i), partialCommand.content, COMMAND_SLOTS);
        //result->content->insert(result->content->end(), partialCommand.content, partialCommand.content + COMMAND_SLOTS);



        // for (uint8_t j = 0; j < this->numPackages.at(key); j++) {
        //     if (partialCommands->at(j)->packageNumber == i) {
        //         PartialCommandMessage partialCommand = *partialCommands->at(j);
        //
        //         if (i == 0) {
        //             // first package saves command and has less content
        //             result->content->insert(result->content->end(), partialCommand.content + 2, partialCommand.content + FIRST_COMMAND_SLOTS);
        //             result->command = (*partialCommand.content)[0];
        //             break;
        //         }
        //
        //         result->content->insert(result->content->end(), partialCommand.content, partialCommand.content + COMMAND_SLOTS);
        //         break;
        //     }
        // }
    }

    return true;
}

