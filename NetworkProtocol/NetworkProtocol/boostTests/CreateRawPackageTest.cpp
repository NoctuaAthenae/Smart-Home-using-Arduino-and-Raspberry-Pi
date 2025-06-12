#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest

#include <boost/test/unit_test.hpp>

#include "../messageObjects.h"


BOOST_AUTO_TEST_SUITE(RawPackage)

BOOST_AUTO_TEST_CASE(CommandRawPackageTest) {

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

        for (int j = 0; j < numberPackages[i]; j++) {

            uint8_t *package = rawPackages.at(j);

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


            package[2] = lastDevive + 1;
            BOOST_CHECK(!Message::checkChecksum(package));
        }
    }
}

BOOST_AUTO_TEST_CASE(AckRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint32_t timestamp = std::rand();
    AcknowledgeMessage msg = AcknowledgeMessage(id, lastDevive, false, false, timestamp);

    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], id);
    BOOST_CHECK_EQUAL(package[4] / 4, 1);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));

    auto* createdMsg = Message::fromRawBytes(rawPackages.at(0));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->nextHop, id);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 1);
    BOOST_CHECK_EQUAL(createdMsg->timestamp, timestamp);

    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}


BOOST_AUTO_TEST_CASE(RegisterRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint32_t timestamp = std::rand();
    uint8_t nextHop = std::rand() % 256;
    RegisterMessage msg = RegisterMessage(id, lastDevive, nextHop, false, false);

    msg.timestamp = timestamp;
    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], nextHop);
    BOOST_CHECK_EQUAL(package[4] / 4, 2);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));


    auto* createdMsg = Message::fromRawBytes(rawPackages.at(0));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->nextHop, nextHop);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 2);

    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}


BOOST_AUTO_TEST_CASE(AcceptRejectRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint32_t timestamp = std::rand();
    uint8_t nextHop = std::rand() % 256;
    AcceptRejectMessage msg = AcceptRejectMessage(id, lastDevive, nextHop, false, false, true);

    msg.timestamp = timestamp;
    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], nextHop);
    BOOST_CHECK_EQUAL(package[4] / 4, 3);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));


    auto* createdMsg = (AcceptRejectMessage*) Message::fromRawBytes(rawPackages.at(0));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->nextHop, nextHop);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 3);
    BOOST_CHECK(createdMsg->isAccept);

    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}


BOOST_AUTO_TEST_CASE(PingRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint32_t timestamp = std::rand();
    uint8_t nextHop = std::rand() % 256;
    PingMessage msg = PingMessage(id, lastDevive, nextHop, false, false, true);
    uint8_t pingId = msg.pingId;

    msg.timestamp = timestamp;
    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], nextHop);
    BOOST_CHECK_EQUAL(package[4] / 4, 4);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));


    auto* createdMsg = (PingMessage*) Message::fromRawBytes(rawPackages.at(0));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->nextHop, nextHop);
    BOOST_CHECK_EQUAL(createdMsg->pingId, pingId);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 4);
    BOOST_CHECK(createdMsg->isResponse);

    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}


BOOST_AUTO_TEST_CASE(RouteCreationRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint32_t timestamp = std::rand();
    uint8_t nextHop = std::rand() % 256;
    uint8_t newId = std::rand() % 256;
    RouteCreationMessage msg = RouteCreationMessage(id, lastDevive, nextHop, false, false, newId);

    msg.timestamp = timestamp;
    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], nextHop);
    BOOST_CHECK_EQUAL(package[4] / 4, 5);
    BOOST_CHECK_EQUAL(package[10], newId);
    BOOST_CHECK_EQUAL(package[11], lastDevive);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));


    auto* createdMsg = (RouteCreationMessage*) Message::fromRawBytes(rawPackages.at(0));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->nextHop, nextHop);
    BOOST_CHECK_EQUAL(createdMsg->newId, newId);
    BOOST_CHECK_EQUAL(createdMsg->route[0], lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->typeAndGroups / 4, 5);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 5);


    createdMsg->lastDeviceId = createdMsg->nextHop;
    uint8_t otherHop = std::rand() % 256;
    createdMsg->nextHop = otherHop;

    uint8_t* secondPackage = createdMsg->getRawPackages().at(0);

    BOOST_CHECK_EQUAL(secondPackage[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(secondPackage[1], id);
    BOOST_CHECK_EQUAL(secondPackage[2], nextHop);
    BOOST_CHECK_EQUAL(secondPackage[3], otherHop);
    BOOST_CHECK_EQUAL(secondPackage[4] / 4, 5);
    BOOST_CHECK_EQUAL(secondPackage[10], newId);
    BOOST_CHECK_EQUAL(secondPackage[11], lastDevive);
    BOOST_CHECK_EQUAL(secondPackage[12], nextHop);

    auto* secondMessage = (RouteCreationMessage*) Message::fromRawBytes(secondPackage);

    BOOST_CHECK_EQUAL(secondMessage->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(secondMessage->receiver, id);
    BOOST_CHECK_EQUAL(secondMessage->lastDeviceId, nextHop);
    BOOST_CHECK_EQUAL(secondMessage->nextHop, otherHop);
    BOOST_CHECK_EQUAL(secondMessage->newId, newId);
    BOOST_CHECK_EQUAL(secondMessage->route[0], lastDevive);
    BOOST_CHECK_EQUAL(secondMessage->route[1], nextHop);
    BOOST_CHECK_EQUAL(secondMessage->getType(), 5);


    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}


BOOST_AUTO_TEST_CASE(AddRemoveToGroupRawPackageTest) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint32_t timestamp = std::rand();
    uint8_t nextHop = std::rand() % 256;
    uint8_t groupId = std::rand() % 256;
    AddRemoveToGroupMessage msg = AddRemoveToGroupMessage(id, lastDevive, nextHop, false, groupId, true);

    msg.timestamp = timestamp;
    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], nextHop);
    BOOST_CHECK_EQUAL(package[4] / 4, 6);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));


    auto* createdMsg = (AddRemoveToGroupMessage*) Message::fromRawBytes(rawPackages.at(0));

    BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    BOOST_CHECK_EQUAL(createdMsg->lastDeviceId, lastDevive);
    BOOST_CHECK_EQUAL(createdMsg->nextHop, nextHop);
    BOOST_CHECK_EQUAL(createdMsg->groupId, groupId);
    BOOST_CHECK_EQUAL(createdMsg->getType(), 6);
    BOOST_CHECK(createdMsg->isAddToGroup);

    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}

BOOST_AUTO_TEST_SUITE_END()