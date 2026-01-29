#ifndef MESSAGEBUILDER_H
#define MESSAGEBUILDER_H
#include <cstdint>
#include <map>

#include "messageObjects.h"

/**
 * Class for building data messages.
 */
class MessageBuilder {
private:
    /**
     * Map of partial data messages.
     * Key: (lastDevice, timestamp)
     * Value: The partial data message.
     */
    std::map<std::pair<uint8_t, uint16_t>, std::vector<PartialDataMessage*>> partialDatas;

    /**
     * Map of the number of packages of a data message.
     * Key: (lastDevice, timestamp)
     * Value: The number of packages.
     */
    std::map<std::pair<uint8_t, uint16_t>, uint8_t> numPackages;

public:
    /**
     * A new partial data message has been received.
     * This function checks if the message is complete, and if so, creates a new data message.
     * The resulting message might have trailing zeros.
     * @param message The received message.
     * @param result The completed data message.
     * @return True if the message is complete and a new data message has been created.
     */
    bool newDataMessage(PartialDataMessage* message, DataMessage* result);

};

#endif //MESSAGEBUILDER_H
