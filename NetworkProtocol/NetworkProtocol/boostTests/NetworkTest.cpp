#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE RawPackageTest


#include <thread>
#include <boost/test/unit_test.hpp>

#include "testNetworkDevice.h"

bool msgsInTransit(std::map<std::pair<uint8_t, uint8_t>, std::vector<uint8_t*>> pipes) {
    for (auto & pipe : pipes) {
        if (!pipe.second.empty()) return true;
    }
    return false;
}


BOOST_AUTO_TEST_SUITE(NetworkTest)

BOOST_AUTO_TEST_CASE(RegistrationTest) {

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

    pipes[std::pair<uint8_t, uint8_t>(3, 6)] = std::vector<uint8_t*>();
    pipes[std::pair<uint8_t, uint8_t>(6, 3)] = std::vector<uint8_t*>();

    std::vector<TestNetworkDevice> devices = {};

    std::map<uint8_t, std::map<uint8_t, std::vector<uint8_t*>*>> ins = {};
    std::map<uint8_t, std::map<uint8_t, std::vector<uint8_t*>*>> outs = {};

    for (auto & pipe : pipes) {
        auto first = pipe.first.first;
        auto second = pipe.first.second;
        if (second == 6 && first == 3) second = 7;
        ins[second][pipe.first.first] = &(pipe.second);
        if (first == 6 && second == 3) first = 7;
        outs[first][pipe.first.second] = &(pipe.second);
    }

    // auto a = new TestNetworkDevice(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(1, 0, 1000, &ins[1], &outs[1]);
    devices.emplace_back(2, 1, 1000, &ins[2], &outs[2]);
    devices.emplace_back(3, 0, 1000, &ins[3], &outs[3]);
    devices.emplace_back(4, 1, 1000, &ins[4], &outs[4]);
    devices.emplace_back(5, 1, 1000, &ins[5], &outs[5]);
    devices.emplace_back(6, 2, 1000, &ins[6], &outs[6]);
    devices.emplace_back(6, 3, 1000, &ins[7], &outs[7]);

    devices[1].registerDevice();


    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 7; i++) {
            devices[i].update();
        }
    }
    BOOST_CHECK(devices[1].isRegistered());
    BOOST_CHECK(!devices[3].isRegistered());

    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));
    devices[3].registerDevice();

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 7; i++) {
            devices[i].update();
        }
    }
    BOOST_CHECK(devices[1].isRegistered());
    BOOST_CHECK(devices[3].isRegistered());

    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));
    devices[2].registerDevice();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));
    devices[4].registerDevice();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));
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

    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));
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

    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));
    devices[7].registerDevice();


    while (msgsInTransit(pipes)) {
        for (int j = 0; j < 8; j++) {
            devices[j].update();
        }
    }

    BOOST_CHECK(!devices[7].isRegistered());


    devices[7].registerDevice();

    for (int j = 0; j < 8; j++) {
        devices[j].update();
    }
    for (int j = 0; j < 8; j++) {
        devices[j].update();
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(2000));

    while (msgsInTransit(pipes)) {
        for (int j = 0; j < 8; j++) {
            devices[j].update();
        }
    }


    BOOST_CHECK(devices[7].isRegistered());
}



BOOST_AUTO_TEST_CASE(DataTest) {

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

    // auto a = new TestNetworkDevice(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(1, 0, 1000, &ins[1], &outs[1]);
    devices.emplace_back(2, 1, 1000, &ins[2], &outs[2]);
    devices.emplace_back(3, 0, 1000, &ins[3], &outs[3]);
    devices.emplace_back(4, 1, 1000, &ins[4], &outs[4]);
    devices.emplace_back(5, 1, 1000, &ins[5], &outs[5]);
    devices.emplace_back(6, 2, 1000, &ins[6], &outs[6]);

    for (int i = 1; i < 7; i++) {
        devices[i].registerDevice();
        while (msgsInTransit(pipes)) {
            for (int j = 0; j < 7; j++) {
                devices[j].update();
            }
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }

    for (int j = 0; j < 7; j++) {
        BOOST_CHECK(devices[j].isRegistered());
    }

    uint8_t data_sizes[] = {20, 54, 128};
    uint8_t number_packages[] = {1, 3, 6};

    for (int i = 0; i < 3; i++) {
        auto data_size = data_sizes[i];

        auto content = new uint8_t[data_size];

        for (int j = 0; j < data_size; j++) {
            content[j] = j;
        }

        devices[2].send(6, content, data_size);

        bool delivered = false;
        while (msgsInTransit(pipes)) {
            for (int j = 0; j < 7; j++) {
                if (devices[j].update()) {
                    BOOST_CHECK(j == 6);
                    uint8_t* received[1];
                    u_int16_t size = devices[j].receive(received);
                    auto receivedData = *received;
                    BOOST_CHECK_EQUAL(size, SLOT_COUNT(number_packages[i]));
                    for (int k = 0; k < data_size; k++) {
                        BOOST_CHECK_EQUAL(content[k], receivedData[k]);
                    }
                    delivered = true;
                }
            }
        }
        BOOST_CHECK(delivered);


        devices[3].send(6, content, data_size);

        delivered = false;
        while (msgsInTransit(pipes)) {
            for (int j = 0; j < 7; j++) {
                if (devices[j].update()) {
                    BOOST_CHECK(j == 6);
                    uint8_t* received[1];
                    u_int16_t size = devices[j].receive(received);
                    auto receivedData = *received;
                    BOOST_CHECK_EQUAL(size, SLOT_COUNT(number_packages[i]));
                    for (int k = 0; k < data_size; k++) {
                        BOOST_CHECK_EQUAL(content[k], receivedData[k]);
                    }
                    delivered = true;
                }
            }
        }
        BOOST_CHECK(delivered);
    }
}

BOOST_AUTO_TEST_CASE(PingTest) {

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

    // auto a = new TestNetworkDevice(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(0, 0, 1000, &ins[0], &outs[0]);
    devices.emplace_back(1, 0, 1000, &ins[1], &outs[1]);
    devices.emplace_back(2, 1, 1000, &ins[2], &outs[2]);
    devices.emplace_back(3, 0, 1000, &ins[3], &outs[3]);
    devices.emplace_back(4, 1, 1000, &ins[4], &outs[4]);
    devices.emplace_back(5, 1, 1000, &ins[5], &outs[5]);
    devices.emplace_back(6, 2, 1000, &ins[6], &outs[6]);

    for (int i = 1; i < 7; i++) {
        devices[i].registerDevice();
        while (msgsInTransit(pipes)) {
            for (int j = 0; j < 7; j++) {
                devices[j].update();
            }
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }

    for (int j = 0; j < 7; j++) {
        BOOST_CHECK(devices[j].isRegistered());
    }



    for (int i = 0; i < 7; i++) {
        for (int k = 0; k < 7; k++) {
            if (i == k) continue;
            uint8_t pingId = devices[i].ping(k);
            std::this_thread::sleep_for(
                std::chrono::milliseconds(2));
            bool delivered = false;
            while (msgsInTransit(pipes)) {
                for (int j = 0; j < 7; j++) {
                    devices[j].update();
                    if (devices[j].checkPing(pingId) > 0) {
                        BOOST_CHECK(j == i);
                        delivered = true;
                    }
                }
            }
            BOOST_CHECK(delivered);
        }
    }
}
BOOST_AUTO_TEST_SUITE_END()
