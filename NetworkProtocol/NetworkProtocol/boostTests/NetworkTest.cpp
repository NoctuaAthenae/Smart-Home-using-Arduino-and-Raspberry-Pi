#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest


#include <boost/test/unit_test.hpp>

#include "testNetworkDevice.h"


BOOST_AUTO_TEST_SUITE(NetworkTest)

BOOST_AUTO_TEST_CASE(NetworkTest) {

    /*
     * Test network
     *
     *          0
     *         / \
     *        1   3
     *       /|\
     *      2 4 5
     *     /
     *    6
     */

    std::map<std::pair<uint8_t, uint8_t>, std::vector<uint8_t*>> pipes = {};

    pipes[std::pair<uint8_t, uint8_t>(0, 1)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(0, 3)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(1, 2)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(1, 4)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(1, 5)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(2, 6)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(1, 0)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(3, 0)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(2, 1)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(4, 1)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(5, 1)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(6, 2)] = std::vector<uint8_t*>();

    std::vector<TestNetworkDevice> devices = {};

    std::map<uint8_t, std::map<uint8_t, std::vector<uint8_t*>*>> ins = {};
    std::map<uint8_t, std::map<uint8_t, std::vector<uint8_t*>*>> outs = {};

    for (auto & pipe : pipes) {
        auto a = ins[pipe.first.first][pipe.first.second];
        ins[pipe.first.second][pipe.first.first] = &(pipe.second);
        outs[pipe.first.first][pipe.first.second] = &(pipe.second);
    }

    //TODO not sending messages

    // auto a = new TestNetworkDevice(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(1, 0, 1000, &ins[1], &outs[1]);
    devices.emplace_back(2, 1, 1000, &ins[2], &outs[2]);
    devices.emplace_back(3, 0, 1000, &ins[3], &outs[3]);
    devices.emplace_back(4, 1, 1000, &ins[4], &outs[4]);
    devices.emplace_back(5, 1, 1000, &ins[5], &outs[5]);
    devices.emplace_back(6, 2, 1000, &ins[6], &outs[6]);

    devices[1].registerDevice();


    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 7; i++) {
            devices[i].update();
        }
    }
    BOOST_CHECK(devices[1].isRegistered());
    BOOST_CHECK(!devices[3].isRegistered());

    sleep(1);
    devices[3].registerDevice();

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 7; i++) {
            devices[i].update();
        }
    }
    BOOST_CHECK(devices[1].isRegistered());
    BOOST_CHECK(devices[3].isRegistered());

    sleep(1);
    devices[2].registerDevice();
    sleep(1);
    devices[4].registerDevice();
    sleep(1);
    devices[5].registerDevice();


    // 1 sends 2 to 0
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(!devices[2].isRegistered());
    BOOST_CHECK(!devices[4].isRegistered());
    BOOST_CHECK(!devices[5].isRegistered());

    // 0 sends to 1 to 2
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(devices[2].isRegistered());
    BOOST_CHECK(!devices[4].isRegistered());
    BOOST_CHECK(!devices[5].isRegistered());

    sleep(1);
    devices[6].registerDevice();

    // 1 sends 4 to 0
    // 6 sends msg to 2
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(!devices[4].isRegistered());
    BOOST_CHECK(!devices[5].isRegistered());
    BOOST_CHECK(!devices[6].isRegistered());

    // 0 sends to 1 to 4
    // 2 sends 6 to 1
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(devices[4].isRegistered());
    BOOST_CHECK(!devices[5].isRegistered());
    BOOST_CHECK(!devices[6].isRegistered());

    // 1 sends 6 to 0
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(!devices[5].isRegistered());
    BOOST_CHECK(!devices[6].isRegistered());

    // 0 sends to 1 to 6
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(!devices[5].isRegistered());
    BOOST_CHECK(devices[6].isRegistered());

    // 1 sends 5 to 0
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }

    BOOST_CHECK(!devices[5].isRegistered());

    // 0 sends to 1 to 2 to 5
    for (int i = 0; i < 7; i++) {
        devices[i].update();
    }



    for (int j = 0; j < 7; j++) {
        BOOST_CHECK(devices[j].isRegistered());
    }


    //
    // uint8_t contentSizes[] = {
    //     FIRST_DATA_PACKAGE_SLOTS,
    //     FIRST_DATA_PACKAGE_SLOTS + 1,
    //     FIRST_DATA_PACKAGE_SLOTS + DATA_SLOTS,
    //     FIRST_DATA_PACKAGE_SLOTS + DATA_SLOTS + 1,
    //     FIRST_DATA_PACKAGE_SLOTS + 2 * DATA_SLOTS,
    //     FIRST_DATA_PACKAGE_SLOTS + 2 * DATA_SLOTS + 1};
    // uint8_t numberPackages[] = {1, 2, 2, 3, 3, 4};
    //
    // for (int i = 0; i < 6; i++) {
    //
    //     uint8_t id = std::rand() % 256;
    //     uint8_t origin = std::rand() % 256;
    //     uint16_t messageId = std::rand() % 65536;
    //
    //     uint8_t *content = new uint8_t[contentSizes[i]];
    //
    //     for (int j = 0; j < contentSizes[i]; j++) {
    //         content[i] = std::rand() % 256;
    //     }
    //
    //     DataMessage msg = DataMessage(id, false, messageId, origin, content, contentSizes[i]);
    //
    //     uint8_t *dataAddress[1];
    //     uint8_t gotNumberPackages = msg.getRawPackages(dataAddress);
    //     uint8_t *rawPackages = *dataAddress;
    //
    //     BOOST_CHECK_EQUAL(gotNumberPackages, numberPackages[i]);
    //
    //     MessageBuilder builder;
    //
    //
    //     for (int j1 = 0; j1 < numberPackages[i]; j1++) {
    //         int j = numberPackages[i] > 1 && j1 < 2 ? 1 - j1 : j1;
    //
    //         uint8_t *package = rawPackages + j * 32;
    //
    //         BOOST_CHECK_EQUAL(package[0], NETWORKPROTOCOL_VERSION);
    //         BOOST_CHECK_EQUAL(package[1], id);
    //         BOOST_CHECK_EQUAL(package[2] / 4, 0);
    //         BOOST_CHECK_EQUAL(package[3], j);
    //         BOOST_CHECK_EQUAL(package[4], origin);
    //
    //         uint16_t createdMessageId = 0;
    //         memcpy(&createdMessageId, package + 5, sizeof(uint16_t));
    //
    //         BOOST_CHECK_EQUAL(createdMessageId, messageId);
    //
    //         auto* createdMsg = dynamic_cast<PartialDataMessage *>(Message::fromRawBytes(rawPackages + j * 32));
    //
    //         BOOST_CHECK_EQUAL(createdMsg->version, NETWORKPROTOCOL_VERSION);
    //         BOOST_CHECK_EQUAL(createdMsg->receiver, id);
    //         BOOST_CHECK_EQUAL(createdMsg->origin, origin);
    //         BOOST_CHECK_EQUAL(createdMsg->messageID, messageId);
    //         BOOST_CHECK_EQUAL(createdMsg->getType(), 0);
    //         BOOST_CHECK_EQUAL(createdMsg->packageNumber, j);
    //
    //         if (j == 0) {
    //             BOOST_CHECK_EQUAL(createdMsg->content[0], numberPackages[i]);
    //             for (int k = 0; k < FIRST_DATA_PACKAGE_SLOTS; k++) {
    //                 BOOST_CHECK_EQUAL(createdMsg->content[FIRST_METADATA_SLOTS + k], content[k]);
    //             }
    //         } else {
    //             int numberSlots = j == numberPackages[i] - 1 ? (contentSizes[i] - FIRST_DATA_PACKAGE_SLOTS) % DATA_SLOTS : DATA_SLOTS;
    //             for (int k = 0; k < numberSlots; k++) {
    //                 BOOST_CHECK_EQUAL(createdMsg->content[k], content[SLOT_COUNT(j) + k]);
    //             }
    //         }
    //
    //
    //         auto createdDataMessage = DataMessage();
    //         bool messageBuilt = builder.newDataMessage(createdMsg, &createdDataMessage);
    //
    //
    //         if (j1 == numberPackages[i] - 1) {
    //             BOOST_CHECK(messageBuilt);
    //             BOOST_CHECK_EQUAL(createdDataMessage.contentSize, SLOT_COUNT(numberPackages[i]));
    //             for (int k = 0; k < contentSizes[i]; k++) {
    //                 BOOST_CHECK_EQUAL(createdDataMessage.content[k], content[k]);
    //             }
    //         } else {
    //             BOOST_CHECK(!messageBuilt);
    //         }
    //     }
    //
    //     Message::cleanUp(rawPackages);
    // }
}

BOOST_AUTO_TEST_SUITE_END()
