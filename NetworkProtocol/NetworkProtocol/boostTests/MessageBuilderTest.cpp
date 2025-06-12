#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest


#include <boost/test/unit_test.hpp>

#include "../messageBuilder.h"
#include "../messageObjects.h"


BOOST_AUTO_TEST_SUITE(MessageBuilderTest)

BOOST_AUTO_TEST_CASE(MessageBuilderTest) {

    uint8_t contentSizes[] = {19, 20, 40, 41, 61, 62};
    uint8_t numberPackages[] = {1, 2, 2, 3, 3, 4};

    for (int i = 0; i < 6; i++) {

        uint8_t id = std::rand() % 256;
        uint8_t lastDevive = std::rand() % 256;
        uint8_t nextHop = std::rand() % 256;
        uint8_t command = std::rand() % 256;
        uint32_t timestamp = std::rand();

        std::vector<uint8_t> content;

        for (int j = 0; j < contentSizes[i]; j++) {
            content.push_back(std::rand() % 256);
        }

        CommandMessage msg = CommandMessage(id, lastDevive, nextHop , false, false, command, &content);

        msg.timestamp = timestamp;

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
            BOOST_CHECK_EQUAL(package[2], lastDevive);
            BOOST_CHECK_EQUAL(package[3], nextHop);
            BOOST_CHECK_EQUAL(package[4] / 4, 0);
            BOOST_CHECK_EQUAL(package[10], j);

            uint32_t createdTimestamp = 0;
            memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

            BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

            BOOST_CHECK(Message::checkChecksum(package));

            auto* createdMsg = (PartialCommandMessage*) Message::fromRawBytes(rawPackages.at(j));

            BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
            BOOST_CHECK_EQUAL(createdMsg->receiver, id);
            BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
            BOOST_CHECK_EQUAL(createdMsg->nextHop, nextHop);
            BOOST_CHECK_EQUAL(createdMsg->getType(), 0);
            BOOST_CHECK_EQUAL(createdMsg->timestamp, timestamp);
            BOOST_CHECK_EQUAL(createdMsg->packageNumber, j);

            if (j == 0) {
                BOOST_CHECK_EQUAL((*createdMsg->content)[0], numberPackages[i]);
                BOOST_CHECK_EQUAL((*createdMsg->content)[1], command);
                for (int k = 0; k < 19; k++) {
                    BOOST_CHECK_EQUAL((*createdMsg->content)[2 + k], content[k]);
                }
            } else {
                int numberSlots = j == numberPackages[i] - 1 ? (contentSizes[i] - 19) % 21 : 21;
                for (int k = 0; k < numberSlots; k++) {
                    BOOST_CHECK_EQUAL((*createdMsg->content)[k], content[19 + (j - 1) * 21 + k]);
                }
            }


            auto createdCommandMessage = CommandMessage();
            bool messageBuilt = builder.newCommandMessage(createdMsg, &createdCommandMessage);

            if (j == numberPackages[i] - 1) {
                BOOST_CHECK(messageBuilt);
                BOOST_CHECK_EQUAL(createdCommandMessage.command, command);
                BOOST_CHECK_EQUAL(createdCommandMessage.content->size(), 19 + 21 * (numberPackages[i] - 1));
                for (int k = 0; k < contentSizes[i]; k++) {
                    BOOST_CHECK_EQUAL((*createdCommandMessage.content)[k], content[k]);
                }
                for (int k = contentSizes[i]; k < (j == 0 ? 19 : 21); k++) {
                    BOOST_CHECK_EQUAL((*createdCommandMessage.content)[k], 0);
                }
            } else {
                BOOST_CHECK(!messageBuilt);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()