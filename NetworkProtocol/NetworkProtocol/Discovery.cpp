#include "Discovery.h"

#include "Messages/messageObjects.h"

bool Discovery::update(uint32_t time, Message** msg) {
    if (this->pingFinished) return this->timer.expired(time);

    *msg = new RegistrationMessage(this->nextDeviceToPing++, false, false, this->id, time, 0, time);

    if (this->nextDeviceToPing == 255) {
        this->pingFinished = true;
        this->timer.start(time);
    }
    return false;
}

void Discovery::newAnswer(uint8_t id, uint8_t level) {
    this->foundDevices.emplace_back(id, level);
}
