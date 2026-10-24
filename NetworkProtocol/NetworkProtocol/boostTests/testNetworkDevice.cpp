//
// Created by artemis on 20/10/2026.
//

#include "testNetworkDevice.h"

#include <iostream>
#include <ostream>


bool TestNetworkDevice::_write(uint8_t *rawPackage, uint8_t nextHop) {
    (*out)[nextHop]->push_back(rawPackage);
    return true;
}

bool TestNetworkDevice::_broadcastToNeighbours(uint8_t *rawPackage, uint8_t exclude) {
    for (auto & it : *out) {
        if (it.first == exclude) continue;
        it.second->push_back(rawPackage);
    }
    return true;
}

uint8_t TestNetworkDevice::_read(uint8_t **rawPackage) {
    for (auto & it : *in) {
        if (!it.second->empty()) {
            *rawPackage = (*it.second)[0];
            it.second->erase(it.second->begin());
            return it.first;
        }
    }
    this->_printError(0, (uint8_t *) "No message found");
    return 0;
}

void TestNetworkDevice::_printError(uint8_t errCode, const uint8_t *msg) {
    auto char_msg = (char *)msg;
    std::cout << "Error: " << errCode << " - " << char_msg << std::endl;
}

bool TestNetworkDevice::_messageAvailable() {
    for (auto & it : *in) {
        if (!it.second->empty()) return true;
    }
    return false;
}

uint32_t TestNetworkDevice::_getTime() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
              duration)
              .count();
}