#ifndef MESSAGEBUILDER_H
#define MESSAGEBUILDER_H
#include <cstdint>
#include <map>

#include "messageObjects.h"

/**
 * Class for building command messages.
 */
class MessageBuilder {
private:
    /**
     * Map of partial command messages.
     * Key: (lastDevice, timestamp)
     * Value: The partial command message.
     */
    std::map<std::pair<uint8_t, uint16_t>, std::vector<PartialCommandMessage*>> partialCommands;

    /**
     * Map of the number of packages of a command message.
     * Key: (lastDevice, timestamp)
     * Value: The number of packages.
     */
    std::map<std::pair<uint8_t, uint16_t>, uint8_t> numPackages;

public:
    /**
     * A new partial command message has been received.
     * This function checks if the message is complete, and if so, creates a new command message.
     * The resulting message might have trailing zeros.
     * @param message The received message.
     * @param result The completed command message.
     * @return True if the message is complete and a new command message has been created.
     */
    bool newCommandMessage(PartialCommandMessage* message, CommandMessage* result);

};

#endif //MESSAGEBUILDER_H
