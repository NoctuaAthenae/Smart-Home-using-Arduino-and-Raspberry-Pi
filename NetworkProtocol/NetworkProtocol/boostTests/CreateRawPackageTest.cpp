#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE CreateRawPackageTest

#include <boost/test/unit_test.hpp>

#include "../messageObjects.h"


BOOST_AUTO_TEST_SUITE(CreateRawPackage)

BOOST_AUTO_TEST_CASE(createAck) {
    uint8_t id = std::rand() % 256;
    uint8_t lastDevive = std::rand() % 256;
    uint8_t messageType = std::rand() % 64;
    uint32_t timestamp = std::rand();
    AcknowledgeMessage msg = AcknowledgeMessage(id, lastDevive, messageType * 4, timestamp);

    std::vector<uint8_t *> rawPackages = msg.getRawPackages();

    BOOST_CHECK_EQUAL(rawPackages.size(), 1);

    uint8_t *package = rawPackages.at(0);

    BOOST_CHECK_EQUAL(package[0], NETWORLPROTOCOL_VERSION);
    BOOST_CHECK_EQUAL(package[1], id);
    BOOST_CHECK_EQUAL(package[2], lastDevive);
    BOOST_CHECK_EQUAL(package[3], id);
    BOOST_CHECK_EQUAL(package[4] / 4, messageType);

    uint32_t createdTimestamp = 0;
    memcpy(&createdTimestamp, package + 6, sizeof(uint32_t));

    BOOST_CHECK_EQUAL(createdTimestamp, timestamp);

    BOOST_CHECK(Message::checkChecksum(package));


    package[2] = lastDevive + 1;
    BOOST_CHECK(!Message::checkChecksum(package));

}

BOOST_AUTO_TEST_SUITE_END()