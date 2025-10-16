#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest


#include <boost/test/unit_test.hpp>

#include "../messageBuilder.h"
#include "../messageObjects.h"


BOOST_AUTO_TEST_SUITE(MessageBuilderTest)

BOOST_AUTO_TEST_CASE(MessageBuilderTest) {

    uint8_t contentSizes[] = {
        FIRST_COMMAND_SLOTS,
        FIRST_COMMAND_SLOTS + 1,
        FIRST_COMMAND_SLOTS + COMMAND_SLOTS,
        FIRST_COMMAND_SLOTS + COMMAND_SLOTS + 1,
        FIRST_COMMAND_SLOTS + 2 * COMMAND_SLOTS,
        FIRST_COMMAND_SLOTS + 2 * COMMAND_SLOTS + 1};
    uint8_t numberPackages[] = {1, 2, 2, 3, 3, 4};

    for (int i = 0; i < 6; i++) {

        uint8_t id = std::rand() % 256;
        uint8_t command = std::rand() % 256;
        uint8_t origin = std::rand() % 256;
        uint16_t messageId = std::rand() % 65536;

        std::vector<uint8_t> content;

        for (int j = 0; j < contentSizes[i]; j++) {
            content.push_back(std::rand() % 256);
        }

        CommandMessage msg = CommandMessage(id, false, false, command, messageId, origin, &content);

        std::vector<uint8_t*> rawPackages = msg.getRawPackages();

        BOOST_CHECK_EQUAL(rawPackages.size(), numberPackages[i]);

        MessageBuilder builder;

        for (int j = 0; j < numberPackages[i]; j++) {

            uint8_t *package = rawPackages.at(j);

            uint8_t packageArray[32];
            for (int k = 0; k < 32; k++) {
                packageArray[k] = package[k];
            }

            BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
            BOOST_CHECK_EQUAL(package[1], id);
            BOOST_CHECK_EQUAL(package[2] / 4, 0);
            BOOST_CHECK_EQUAL(package[3], j);
            BOOST_CHECK_EQUAL(package[4], origin);

            uint16_t createdMessageId = 0;
            memcpy(&createdMessageId, package + 5, sizeof(uint16_t));

            BOOST_CHECK_EQUAL(createdMessageId, messageId);

            auto* createdMsg = dynamic_cast<PartialCommandMessage *>(Message::fromRawBytes(rawPackages.at(j)));

            BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
            BOOST_CHECK_EQUAL(createdMsg->receiver, id);
            BOOST_CHECK_EQUAL(createdMsg->origin, origin);
            BOOST_CHECK_EQUAL(createdMsg->messageID, messageId);
            BOOST_CHECK_EQUAL(createdMsg->getType(), 0);
            BOOST_CHECK_EQUAL(createdMsg->packageNumber, j);

            if (j == 0) {
                BOOST_CHECK_EQUAL(createdMsg->content[0], command);
                BOOST_CHECK_EQUAL(createdMsg->content[1], numberPackages[i]);
                for (int k = 0; k < FIRST_COMMAND_SLOTS; k++) {
                    BOOST_CHECK_EQUAL(createdMsg->content[2 + k], content[k]);
                }
            } else {
                int numberSlots = j == numberPackages[i] - 1 ? (contentSizes[i] - FIRST_COMMAND_SLOTS) % COMMAND_SLOTS : COMMAND_SLOTS;
                for (int k = 0; k < numberSlots; k++) {
                    BOOST_CHECK_EQUAL(createdMsg->content[k], content[SLOT_COUNT(j) + k]);
                }
            }


            auto createdCommandMessage = CommandMessage();
            bool messageBuilt = builder.newCommandMessage(createdMsg, &createdCommandMessage);

            if (j == numberPackages[i] - 1) {
                BOOST_CHECK(messageBuilt);
                BOOST_CHECK_EQUAL(createdCommandMessage.command, command);
                BOOST_CHECK_EQUAL(createdCommandMessage.content->size(), SLOT_COUNT(numberPackages[i]));
                for (int k = 0; k < contentSizes[i]; k++) {
                    BOOST_CHECK_EQUAL((*createdCommandMessage.content)[k], content[k]);
                }
            } else {
                BOOST_CHECK(!messageBuilt);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()