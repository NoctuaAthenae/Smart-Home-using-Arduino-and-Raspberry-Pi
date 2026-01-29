#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest


#include <boost/test/unit_test.hpp>

#include "../Messages/messageBuilder.h"
#include "../Messages/messageObjects.h"


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

        uint8_t *content = new uint8_t[contentSizes[i]];

        for (int j = 0; j < contentSizes[i]; j++) {
            content[i] = std::rand() % 256;
        }

        CommandMessage msg = CommandMessage(id, false, false, command, messageId, origin, content, contentSizes[i]);

        uint8_t *dataAddress[1];
        uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
        uint8_t *rawPackages = *dataAddress;

        BOOST_CHECK_EQUAL(gotNumberPackages, numberPackages[i]);

        MessageBuilder builder;


        for (int j1 = 0; j1 < numberPackages[i]; j1++) {
            int j = numberPackages[i] > 1 && j1 < 2 ? 1 - j1 : j1;

            uint8_t *package = rawPackages + j * 32;

            BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
            BOOST_CHECK_EQUAL(package[1], id);
            BOOST_CHECK_EQUAL(package[2] / 4, 0);
            BOOST_CHECK_EQUAL(package[3], j);
            BOOST_CHECK_EQUAL(package[4], origin);

            uint16_t createdMessageId = 0;
            memcpy(&createdMessageId, package + 5, sizeof(uint16_t));

            BOOST_CHECK_EQUAL(createdMessageId, messageId);

            auto* createdMsg = dynamic_cast<PartialCommandMessage *>(Message::fromRawBytes(rawPackages + j * 32));

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


            if (j1 == numberPackages[i] - 1) {
                BOOST_CHECK(messageBuilt);
                BOOST_CHECK_EQUAL(createdCommandMessage.command, command);
                BOOST_CHECK_EQUAL(createdCommandMessage.contentSize, SLOT_COUNT(numberPackages[i]));
                for (int k = 0; k < contentSizes[i]; k++) {
                    BOOST_CHECK_EQUAL(createdCommandMessage.content[k], content[k]);
                }
            } else {
                BOOST_CHECK(!messageBuilt);
            }
        }

        Message::cleanUp(rawPackages);
    }
}

BOOST_AUTO_TEST_SUITE_END()