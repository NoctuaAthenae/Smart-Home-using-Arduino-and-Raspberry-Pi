#include "ConnectionBenchmarkWrapper.h"

#include "../Messages/messageObjects.h"
#include "../networkDevice.h"

bool ConnectionBenchmarkWrapper::update(uint32_t time, Message** msg) {

    if (this->currentDeviceIndex == this->numberDevices) {
        // All messages to all devices have already been sent, check if all benchmarks have finished

        for (uint8_t i = 0; i < this->numberDevices; ++i) {
            uint8_t id = (*this->devices)[i];

            // A benchmark has not finished yet
            if (!this->benchmarks[id]->update(time)) return false;
        }

        return true;
    }

    uint8_t id = (*this->devices)[this->currentDeviceIndex];
    *msg = new PingMessage(id, time % 256, this->id, false, time);

    if (++this->numberMessagesSent == this->numberMessages) {
        this->numberMessagesSent = 0;
        ++this->currentDeviceIndex;
    }

    return false;
}

void ConnectionBenchmarkWrapper::newAnswer(uint8_t id, uint16_t rtt) {
    this->benchmarks[id]->newAnswer(rtt);
}