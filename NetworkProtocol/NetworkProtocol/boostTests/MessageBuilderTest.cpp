#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest


#include <boost/test/unit_test.hpp>

#include "../Messages/messageBuilder.h"
#include "../Messages/messageObjects.h"


BOOST_AUTO_TEST_SUITE(MessageBuilderTest)

BOOST_AUTO_TEST_CASE(MessageBuilderTest) {

    uint8_t contentSizes[] = {
        FIRST_DATA_PACKAGE_SLOTS,
        FIRST_DATA_PACKAGE_SLOTS + 1,
        FIRST_DATA_PACKAGE_SLOTS + DATA_SLOTS,
        FIRST_DATA_PACKAGE_SLOTS + DATA_SLOTS + 1,
        FIRST_DATA_PACKAGE_SLOTS + 2 * DATA_SLOTS,
        FIRST_DATA_PACKAGE_SLOTS + 2 * DATA_SLOTS + 1};
    uint8_t numberPackages[] = {1, 2, 2, 3, 3, 4};

    for (int i = 0; i < 6; i++) {

        uint8_t id = std::rand() % 256;
        uint8_t origin = std::rand() % 256;
        uint16_t messageId = std::rand() % 65536;

        uint8_t *content = new uint8_t[contentSizes[i]];

        for (int j = 0; j < contentSizes[i]; j++) {
            content[i] = std::rand() % 256;
        }

        DataMessage msg = DataMessage(id, false, messageId, origin, content, contentSizes[i]);

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

            auto* createdMsg = dynamic_cast<PartialDataMessage *>(Message::fromRawBytes(rawPackages + j * 32));

            BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
            BOOST_CHECK_EQUAL(createdMsg->receiver, id);
            BOOST_CHECK_EQUAL(createdMsg->origin, origin);
            BOOST_CHECK_EQUAL(createdMsg->messageID, messageId);
            BOOST_CHECK_EQUAL(createdMsg->getType(), 0);
            BOOST_CHECK_EQUAL(createdMsg->packageNumber, j);

            if (j == 0) {
                BOOST_CHECK_EQUAL(createdMsg->content[0], numberPackages[i]);
                for (int k = 0; k < FIRST_DATA_PACKAGE_SLOTS; k++) {
                    BOOST_CHECK_EQUAL(createdMsg->content[FIRST_METADATA_SLOTS + k], content[k]);
                }
            } else {
                int numberSlots = j == numberPackages[i] - 1 ? (contentSizes[i] - FIRST_DATA_PACKAGE_SLOTS) % DATA_SLOTS : DATA_SLOTS;
                for (int k = 0; k < numberSlots; k++) {
                    BOOST_CHECK_EQUAL(createdMsg->content[k], content[SLOT_COUNT(j) + k]);
                }
            }


            auto createdDataMessage = DataMessage();
            bool messageBuilt = builder.newDataMessage(createdMsg, &createdDataMessage);


            if (j1 == numberPackages[i] - 1) {
                BOOST_CHECK(messageBuilt);
                BOOST_CHECK_EQUAL(createdDataMessage.contentSize, SLOT_COUNT(numberPackages[i]));
                for (int k = 0; k < contentSizes[i]; k++) {
                    BOOST_CHECK_EQUAL(createdDataMessage.content[k], content[k]);
                }
            } else {
                BOOST_CHECK(!messageBuilt);
            }
        }

        Message::cleanUp(rawPackages);
    }
}

BOOST_AUTO_TEST_SUITE_END()