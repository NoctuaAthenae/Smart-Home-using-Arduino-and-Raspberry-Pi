#include "networkDevice.h"

#include <algorithm>

#include "Messages/messageObjects.h"

#define DISCOVERY_CHANNEL 255

bool NetworkDevice::_assembleAndSend(uint8_t receiver, bool group, bool groupAscending, uint8_t *data, uint16_t dataSize) {
    auto message = DataMessage(receiver, group, groupAscending, this->_getMessageID(),
        this->id, data, dataSize);

    return this->_sendInternal(&message);
}

bool NetworkDevice::_sendInternal(Message *message, uint8_t sender) {

    if (!message->isGroup()) {
        uint8_t nextHop = 0;
        if (this->routingTable.count(message->receiver) > 0) {
            nextHop = this->routingTable.at(message->receiver);
        } else {
            nextHop = this->parent;
        }
        return this->_write(message, nextHop);
    }


    bool sendingSuccessful = true;

    // group messages are sent to all children and parent, that is not the sender
    for (const uint8_t i : this->children) {
        if (i != 0 && i != sender && !_write(message, i)) {
            sendingSuccessful = false;
        }
    }
    if (this->parent != sender && !_write(message, this->parent)) {
        sendingSuccessful = false;
    }
    return sendingSuccessful;
}

bool NetworkDevice::_processMessage(Message *message, uint8_t sender) {
    // TODO commands must set members got by receive method

    switch (message->getType()) {
        case 0: {   // data message
            auto *dataMessage = dynamic_cast<DataMessage *>(message);

            // delete previous params and save the new
            delete this->lastData;
            this->lastData = new uint8_t[dataMessage->contentSize];
            memcpy(this->lastData, dataMessage->content, dataMessage->contentSize);
            this->lastDataSize = dataMessage->contentSize;
            return true;
        }
        case 1: {   // registration message
            auto *registrationMsg = dynamic_cast<RegistrationMessage *>(message);
            switch (registrationMsg->registrationType) {
                case 0: {   // discovery
                    if (registrationMsg->extraField != 255) {
                        // Got an answer to own discovery
                        this->discovery->newAnswer(registrationMsg->newDeviceID, registrationMsg->extraField);
                        return false;
                    }
                    // Other device discovers
                    uint8_t childSlot = 4;
                    for (int i = 0; i < 4; ++i) {
                        if (this->children[i] == 0) {
                            childSlot = i;
                            break;
                        }
                    }
                    // cannot accept more children
                    if (childSlot == 4) return false;

                    // TODO figure out how to address discoveries
                    registrationMsg->receiver = DISCOVERY_CHANNEL;
                    registrationMsg->extraField = this->hierarchyLevel;
                    this->_sendInternal(registrationMsg);
                    break;
                }
                case 1: {   // registration request
                    // new device wants to register with this as a parent
                    if (registrationMsg->receiver != this->id) return false;
                    this->tempRoutingTable[registrationMsg->tempID] = 0;
                    registrationMsg->receiver = 0;
                    registrationMsg->registrationType = 2;
                    this->_sendInternal(registrationMsg);

                    break;
                }
                case 2: {   // route creation
                    // new device as a descendant node
                    if (this->id == 0) {
                        if (registrationMsg->newDeviceID == 0 || this->routingTable.count(registrationMsg->newDeviceID) == 0) {
                            // if the device id is not set, assign first free ID
                            RegistrationMessage answerMsg = RegistrationMessage(0, false,
                                false, 0, registrationMsg->tempID, 3, false);
                            for (uint8_t i = 0; true; ++i) {
                                if (this->routingTable.count(i) < 1) {
                                    answerMsg.receiver = i;
                                    answerMsg.newDeviceID = i;
                                    answerMsg.extraField = true;
                                    break;
                                }
                                if (i == 255) break;
                            }
                            this->_sendInternal(&answerMsg);
                        } else {

                            // if the device ID is set and there is a device with the ID already in the routing table,
                            // the hub has to ping id and wait for timeout, then accept
                            uint32_t time = this->_getTime();
                            this->registrationPings.push_back({registrationMsg->newDeviceID, Timer(this->timeout).start(time), registrationMsg->tempID});
                            PingMessage pingMsg = PingMessage(registrationMsg->newDeviceID, false, false,
                                time % 256, this->id, false, time);
                            this->_sendInternal(&pingMsg);
                        }
                        this->tempRoutingTable[registrationMsg->tempID] = sender;
                    } else {
                        this->tempRoutingTable[registrationMsg->tempID] = sender;
                        this->_sendInternal(registrationMsg);
                    }
                    break;
                }
                case 3: {   // registration response
                    // registration of a device has been accepted or rejected
                    if (this->tempID == registrationMsg->tempID) {
                        // this device is accepted/rejected
                        if (!registrationMsg->extraField) {
                            return false;
                        }
                        this->registered = true;
                        this->id = registrationMsg->newDeviceID;
                        return false;
                    }

                    // temporarily update the routing table to the new path
                    uint8_t originalNextHop = 0;
                    bool routeExists = this->routingTable.find(registrationMsg->newDeviceID) != this->routingTable.end();
                    if (routeExists) originalNextHop = this->routingTable[registrationMsg->newDeviceID];

                    this->routingTable[registrationMsg->newDeviceID] = this->tempRoutingTable[registrationMsg->tempID];
                    this->tempRoutingTable.erase(registrationMsg->tempID);

                    this->_sendInternal(registrationMsg);

                    // if the device was rejected, restore the old path
                    if (!registrationMsg->extraField) {
                        if (routeExists) {
                            this->routingTable[registrationMsg->newDeviceID] = originalNextHop;
                        } else {
                            this->routingTable.erase(registrationMsg->newDeviceID);
                        }
                    }
                    break;
                }
            }
            break;
        }
        case 2: {   // ping message
            if (message->receiver != this->id) return false;
            auto *pingMsg = dynamic_cast<PingMessage *>(message);
            if (!pingMsg->isResponse) {
                pingMsg->isResponse = true;
                pingMsg->receiver = pingMsg->senderId;
                pingMsg->senderId = this->id;
                this->_sendInternal(pingMsg);
                return false;
            }

            this->pings[pingMsg->senderId].responseTime = Timer::elapsed(pingMsg->timestamp, this->_getTime());


            break;
        }
        case 3: {   // add/remove to group message
            if (message->receiver != this->id) return false;
            auto *groupMsg = dynamic_cast<AddRemoveToGroupMessage *>(message);
            if (groupMsg->isAddToGroup) {
                this->groups.push_back(groupMsg->groupId);
                return false;
            }
            this->groups.erase(std::remove(this->groups.begin(), this->groups.end(), groupMsg->groupId),
                this->groups.end());

            break;
        }
        case 4: {   // error message
            if (this->id == 1) {
                auto *errMsg = dynamic_cast<ErrorMessage *>(message);
                this->_printError(errMsg->errorCode, errMsg->erroneousMessage);
                return false;
            }

            _sendInternal(message);
            break;
        }
        case 5: {   // disconnect message
            auto *connectionMsg = dynamic_cast<ReDisconnectMessage *>(message);
            if (connectionMsg->receiver == this->id) {
                if (connectionMsg->isDisconnect) {
                    this->registered = false;
                }
                return false;
            }

            if (connectionMsg->isDisconnect) {
                this->routingTable.erase(connectionMsg->receiver);
                this->_sendInternal(connectionMsg);
                return false;
            }

            // Reconnect message
            if (this->routingTable.count(connectionMsg->receiver) > 0) {
                // there is a path from this node to the reconnecting node, so deconstruct this path
                connectionMsg->isDisconnect = true;
            }
            // send the message on the old path, before updating the path
            this->_sendInternal(connectionMsg);
            this->routingTable[connectionMsg->receiver] = sender;
            break;
        }
    }
    return false;
}

uint8_t NetworkDevice::_getMessageID() {
    return this->nextID++;
}

bool NetworkDevice::isInGroup(uint8_t group) {
    for (uint8_t i : groups) {
        if (i == group) {
            return true;
        }
    }
    return false;
}

bool NetworkDevice::registerDevice() {
    uint8_t lowestLevel = 255;
    uint8_t foundParent = 255;
    for (auto pair: this->discovery->foundDevices) {
        if (pair.second < lowestLevel) {
            lowestLevel = pair.second;
            foundParent = pair.first;
        }
    }
    if (foundParent == 255) return false;

    this->tempID = this->_getTime();

    auto msg = new RegistrationMessage(foundParent, false, false, this->id, this->tempID, 1);
    this->_sendInternal(msg);
    delete msg;
    return true;
}

void NetworkDevice::startBenchmark() {
    // Check for a better connection with benchmarks

    uint8_t numberDevices = this->discovery->foundDevices.size();

    auto *devices = new uint8_t[numberDevices];

    for (uint8_t i = 0; i < numberDevices; ++i) {
        devices[i] = this->discovery->foundDevices[i].first;
    }

    this->benchmark_wrapper = new ConnectionBenchmarkWrapper(&devices, numberDevices, 50, 1000, this->_getTime(), this->id);

}

bool NetworkDevice::update() {

    if (this->discovery != nullptr) {

        Message* messageAddress[1];
        *messageAddress = nullptr;
        bool finished = this->discovery->update(this->_getTime(), messageAddress);
        if (finished) {
            if (this->registered) this->startBenchmark();
            else this->registerDevice();

            delete this->discovery;
            this->discovery = nullptr;
        } else if (*messageAddress != nullptr) {
            this->_sendInternal(*messageAddress);
            delete *messageAddress;
        }
    }

    if (this->benchmark_wrapper != nullptr) {
        Message* messageAddress[1];
        *messageAddress = nullptr;
        bool finished = this->benchmark_wrapper->update(this->_getTime(), messageAddress);
        if (finished) {

            delete this->benchmark_wrapper;
            this->benchmark_wrapper = nullptr;
        } else if (*messageAddress != nullptr) {
            this->_sendInternal(*messageAddress);
            delete *messageAddress;
        }
    }

    auto time = this->_getTime();

    // hub checks pending registration pings for timeouts
    // other devices do not add elements to the vector, so an if clause is not needed
    for (auto ping : this->registrationPings) {
        if (ping.timer.expired(time)) {
            this->registrationPings.erase(std::remove(this->registrationPings.begin(), this->registrationPings.end(), ping),
                this->registrationPings.end());

            // The ping for an ID a device is trying to register with has timed out, so send a disconnect message on the old path
            ReDisconnectMessage msg = ReDisconnectMessage(ping.newDeviceID, false, false, true);
            this->_sendInternal(&msg);
            // then update the routing table
            this->routingTable[ping.newDeviceID] = this->tempRoutingTable[ping.tempID];
            this->tempRoutingTable.erase(ping.tempID);
            // and accept the registration request
            RegistrationMessage answerMsg = RegistrationMessage(ping.newDeviceID, false, false, ping.newDeviceID, ping.tempID, 3, true);
            this->_sendInternal(&answerMsg);
        }
    }

    if (!_messageAvailable()) return false;

    Message* messageAddress[1];
    uint8_t sender = this->_read(messageAddress);
    Message* message = *messageAddress;

    if (message->isGroup()) {
        // group messages are always broadcasted
        this->_sendInternal(message, sender);
        // group message cannot contain messages that hops on the way need to process
        if (this->isInGroup(message->receiver)) {
            return this->_processMessage(message, sender);
        }
    } else {
        // forward data messages if this is not the receiver (including broadcasts)
        if (message->getType() == 0 && message->receiver != this->id) {
            this->_sendInternal(message, sender);
            return false;
        }
        // message might contain info this device needs even if device is not the receiver
        return this->_processMessage(message, sender);
    }

    delete message;

    return false;
}

bool NetworkDevice::send(uint8_t receiver, uint8_t *data, uint16_t dataSize) {
    return this->_assembleAndSend(receiver, false, false, data, dataSize);
}

bool NetworkDevice::sendToGroup(uint8_t group, uint8_t *data, uint16_t dataSize) {
    return this->_assembleAndSend(group, true, false, data, dataSize);
}

uint16_t NetworkDevice::receive(uint8_t **data) {
    *data = this->lastData;
    return this->lastDataSize;
}

uint8_t NetworkDevice::ping(uint8_t targetID) {
    uint8_t pingID = this->_getMessageID();
    PingMessage pingMsg = PingMessage(targetID, false, false, pingID, this->id, false, this->_getTime());
    this->_sendInternal(&pingMsg);
    return pingID;
}

uint32_t NetworkDevice::checkPing(uint8_t pingID)  {
    uint32_t responseTime = this->pings[pingID].responseTime;
    if (responseTime > 0) {
        this->pings.erase(pingID);
    }
    return responseTime;
}
