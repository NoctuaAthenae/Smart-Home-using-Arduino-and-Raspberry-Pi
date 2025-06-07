#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest

#include <boost/test/unit_test.hpp>

#include "../messageObjects.h"


BOOST_AUTO_TEST_SUITE(RawPackage)

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

BOOST_AUTO_TEST_SUITE_END()