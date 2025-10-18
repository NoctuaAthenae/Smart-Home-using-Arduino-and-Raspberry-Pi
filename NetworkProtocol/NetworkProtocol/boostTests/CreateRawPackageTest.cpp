#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest

#include <boost/test/unit_test.hpp>
#include <cstdlib>

#include "../messageObjects.h"


BOOST_AUTO_TEST_SUITE(RawPackage)

BOOST_AUTO_TEST_CASE(CommandRawPackageTest) {

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

        for (int j = 0; j < numberPackages[i]; j++) {

            uint8_t *package = rawPackages + j * 32;

            BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
            BOOST_CHECK_EQUAL(package[1], id);
            BOOST_CHECK_EQUAL(package[2] / 4, 0);
            BOOST_CHECK_EQUAL(package[3], j);
            BOOST_CHECK_EQUAL(package[4], origin);

            uint16_t createdMessageId = 0;
            memcpy(&createdMessageId, package + 5, sizeof(uint16_t));

            BOOST_CHECK_EQUAL(createdMessageId, messageId);

            if (j == 0) {
                BOOST_CHECK_EQUAL(package[7], command);
                BOOST_CHECK_EQUAL(package[8], numberPackages[i]);
                for (int k = 0; k < FIRST_COMMAND_SLOTS; k++) {
                    BOOST_CHECK_MESSAGE(package[9 + k] == content[k], "Package 0 at slot " << k);
                }
            } else {
                int numberSlots = j == numberPackages[i] - 1 ? (contentSizes[i] - FIRST_COMMAND_SLOTS) % COMMAND_SLOTS : COMMAND_SLOTS;
                for (int k = 0; k < numberSlots; k++) {
                    BOOST_CHECK_MESSAGE(package[7 + k] == content[SLOT_COUNT(j) + k],
                        "Package " << j << " at slot " << k);
                }
            }

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
                    BOOST_CHECK_MESSAGE(createdMsg->content[2 + k] == content[k], "Package 0 at slot " << k);
                }
            } else {
                int numberSlots = j == numberPackages[i] - 1 ? (contentSizes[i] - FIRST_COMMAND_SLOTS) % COMMAND_SLOTS : COMMAND_SLOTS;
                for (int k = 0; k < numberSlots; k++) {
                    BOOST_CHECK_MESSAGE(createdMsg->content[k] == content[SLOT_COUNT(j) + k],
                        "Package " << j << " at slot " << k);
                }
            }

            delete createdMsg;
        }

        Message::cleanUp(rawPackages);
    }
}


BOOST_AUTO_TEST_CASE(RegisterRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t newDeviceID = std::rand() % 256;
    RegisterMessage msg = RegisterMessage(id, false, false, newDeviceID);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 1);
    BOOST_CHECK_EQUAL(package[3], newDeviceID);

    auto* createdMsg = dynamic_cast<RegisterMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 1);
    BOOST_CHECK_EQUAL(createdMsg->newDeviceID, newDeviceID);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}

BOOST_AUTO_TEST_CASE(RegisterRawPackageTempIdTest) {
    uint8_t id = std::rand() % 256;
    uint32_t tempId = std::rand();
    RegisterMessage msg = RegisterMessage(id, false, false, tempId);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 1);
    BOOST_CHECK_EQUAL(package[3], 0);

    uint32_t createdId = 0;
    memcpy(&createdId, package + 4, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdId, tempId);


    auto* createdMsg = dynamic_cast<RegisterMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 1);
    BOOST_CHECK_EQUAL(createdMsg->newDeviceID, 0);
    BOOST_CHECK_EQUAL(createdMsg->tempID, tempId);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}


BOOST_AUTO_TEST_CASE(AcceptRejectRawPackageTest) {
    uint8_t id = std::rand() % 256;
    AcceptRejectMessage msg = AcceptRejectMessage(id, false, false, true);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 2);
    BOOST_CHECK(package[3]);


    auto* createdMsg = dynamic_cast<AcceptRejectMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 2);
    BOOST_CHECK(createdMsg->isAccept);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}

BOOST_AUTO_TEST_CASE(AcceptRejectTempIdRawPackageTest) {
    uint32_t tempId = std::rand();
    AcceptRejectMessage msg = AcceptRejectMessage(0, false, false, true, tempId);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], 0);
    BOOST_CHECK_EQUAL(package[2] / 4, 2);
    BOOST_CHECK(package[3]);

    uint32_t createdId = 0;
    memcpy(&createdId, package + 4, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdId, tempId);


    auto* createdMsg = dynamic_cast<AcceptRejectMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, 0);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 2);
    BOOST_CHECK_EQUAL(createdMsg->tempID, tempId);
    BOOST_CHECK(createdMsg->isAccept);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}


BOOST_AUTO_TEST_CASE(PingRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t pingID = std::rand() % 256;
    uint8_t senderID = std::rand() % 256;
    uint32_t timestamp = std::rand();
    PingMessage msg = PingMessage(id, false, false, pingID, senderID, false, timestamp);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 3);
    BOOST_CHECK_EQUAL(package[3], senderID);
    BOOST_CHECK_EQUAL(package[4], pingID);
    BOOST_CHECK(!package[5]);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);


    auto* createdMsg = dynamic_cast<PingMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->senderId, senderID);
    BOOST_CHECK_EQUAL(createdMsg->pingId, pingID);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 3);
    BOOST_CHECK(!createdMsg->isResponse);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}


BOOST_AUTO_TEST_CASE(RouteCreationRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t newDeviceID = std::rand() % 256;
    RouteCreationMessage msg = RouteCreationMessage(id, false, false, newDeviceID);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 4);
    BOOST_CHECK_EQUAL(package[3], newDeviceID);


    auto* createdMsg = dynamic_cast<RouteCreationMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->newDeviceID, newDeviceID);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 4);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}

BOOST_AUTO_TEST_CASE(RouteCreationTempIDRawPackageTest) {
    uint32_t tempID = std::rand();
    RouteCreationMessage msg = RouteCreationMessage(0, false, false, tempID);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], 0);
    BOOST_CHECK_EQUAL(package[2] / 4, 4);
    BOOST_CHECK_EQUAL(package[3], 0);

    uint32_t createdID = 0;
    memcpy(&createdID, package + 4, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdID, tempID);


    auto* createdMsg = dynamic_cast<RouteCreationMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, 0);
    BOOST_CHECK_EQUAL(createdMsg->newDeviceID, 0);
    BOOST_CHECK_EQUAL(createdMsg->tempID, tempID);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 4);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}


BOOST_AUTO_TEST_CASE(AddRemoveToGroupRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t groupId = std::rand() % 256;
    AddRemoveToGroupMessage msg = AddRemoveToGroupMessage(id, false, groupId, true);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 5);
    BOOST_CHECK(package[3]);
    BOOST_CHECK_EQUAL(package[4], groupId);


    auto* createdMsg = dynamic_cast<AddRemoveToGroupMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->groupId, groupId);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 5);
    BOOST_CHECK(createdMsg->isAddToGroup);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}


BOOST_AUTO_TEST_CASE(ErrorRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t errorCode = std::rand() % 256;

    uint8_t erroneousMessage[28];

    for (unsigned char & i : erroneousMessage) {
        i = std::rand() % 256;
    }

    ErrorMessage msg = ErrorMessage(id, false, false, errorCode, erroneousMessage);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 6);
    BOOST_CHECK_EQUAL(package[3], errorCode);

    for (int i = 0; i < 28; ++i) {
        BOOST_CHECK_EQUAL(package[i + 4], erroneousMessage[i]);
    }


    auto* createdMsg = dynamic_cast<ErrorMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->errorCode, errorCode);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 6);

    for (int i = 0; i < 28; ++i) {
        BOOST_CHECK_EQUAL(createdMsg->erroneousMessage[i], erroneousMessage[i]);
    }

    delete createdMsg;
    Message::cleanUp(rawPackages);
}

BOOST_AUTO_TEST_CASE(DiscoverRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t newDeviceID = std::rand() % 256;
    DiscoverMessage msg = DiscoverMessage(id, false, false, newDeviceID);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 7);
    BOOST_CHECK(!package[3]);
    BOOST_CHECK_EQUAL(package[4], newDeviceID);


    auto* createdMsg = dynamic_cast<DiscoverMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->ID, newDeviceID);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 7);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}

BOOST_AUTO_TEST_CASE(DiscoverTempIDRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint32_t tempID = std::rand();
    DiscoverMessage msg = DiscoverMessage(id, false, false, tempID);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 7);
    BOOST_CHECK(!package[3]);
    BOOST_CHECK_EQUAL(package[4], 0);

    uint32_t createdID = 0;
    memcpy(&createdID, package + 5, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdID, tempID);


    auto* createdMsg = dynamic_cast<DiscoverMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->ID, 0);
    BOOST_CHECK_EQUAL(createdMsg->tempID, tempID);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 7);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}


BOOST_AUTO_TEST_CASE(ReDisconnectRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t groupId = std::rand() % 256;
    ReDisconnectMessage msg = ReDisconnectMessage(id, false, groupId, true);

    uint8_t *dataAddress[1];
    uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    uint8_t *rawPackages = *dataAddress;

    BOOST_CHECK_EQUAL(gotNumberPackages, 1);

    uint8_t *package = rawPackages;

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2] / 4, 8);
    BOOST_CHECK(package[3]);


    auto* createdMsg = dynamic_cast<ReDisconnectMessage *>(Message::fromRawBytes(rawPackages));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 8);
    BOOST_CHECK(createdMsg->isDisconnect);

    delete createdMsg;
    Message::cleanUp(rawPackages);
}

BOOST_AUTO_TEST_SUITE_END()