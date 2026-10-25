#include "networkDevice.h"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <ostream>

#include "Messages/messageObjects.h"

bool NetworkDevice::_assembleAndSend(uint8_t receiver, bool group, uint8_t *data, uint16_t dataSize) {
    auto message = DataMessage(receiver, group, this->_getMessageID(),
        this->id, data, dataSize);

    return this->_routeAndSend(&message);
}

bool NetworkDevice::_routeAndSend(Message *message, uint8_t sender) {

    if (!message->group) {
        uint8_t nextHop = 0;
        if (this->routingTable.count(message->receiver) > 0) {
            nextHop = this->routingTable.at(message->receiver);
        } else {
            if (message->getType() == 1) {
                auto *registrationMsg = dynamic_cast<RegistrationMessage *>(message);
                if (registrationMsg->registrationType == 3 && this->tempRoutingTable.count(registrationMsg->tempID) > 0) {
                    nextHop = this->tempRoutingTable.at(registrationMsg->tempID);
                    if (nextHop == 0) {
                        // one above new endpoint
                        return this->_sendInternal(message, sender, true);
                    }
                    return this->_sendInternal(message, nextHop);
                }
            }
            nextHop = this->parent;
        }
        return this->_sendInternal(message, nextHop);
    }

    return this->_sendInternal(message, sender, true);
}

bool NetworkDevice::_sendInternal(Message *message, uint8_t nextHop, bool broadcast) {
    uint8_t *dataAddress[1];
    uint8_t number_packages = message->getRawPackages(dataAddress);
    uint8_t *packages = *dataAddress;
    for (int i = 0; i < number_packages; i++) {
        if (broadcast) {
            if (!this->_broadcastToNeighbours(packages + i * 32, nextHop)) {
                return false;
            }
        } else {
            if (!this->_write(packages + i * 32, nextHop)) {
                return false;
            }
        }
    }
    return true;
}

bool NetworkDevice::_processMessage(Message *message, uint8_t sender) {
    // TODO commands must set members got by receive method

    switch (message->getType()) {
        case 0: {   // data message
            if (!this->isRegistered()) return false;
            auto *dataMessage = dynamic_cast<PartialDataMessage *>(message);

            DataMessage result[1];
            if (!this->message_builder.newDataMessage(dataMessage, result)) return false;

            // delete previous params and save the new
            delete this->lastData;
            this->lastData = new uint8_t[result->contentSize];
            memcpy(this->lastData, result->content, result->contentSize);
            this->lastDataSize = result->contentSize;
            return true;
        }
        case 1: {   // registration message
            auto *registrationMsg = dynamic_cast<RegistrationMessage *>(message);
            switch (registrationMsg->registrationType) {
                case 1: {   // registration request
                    // new device wants to register with this as a parent
                    if (!this->isRegistered()) return false;
                    if (registrationMsg->receiver != this->id) return false;

                    this->tempRoutingTable[registrationMsg->tempID] = 0;
                    if (this->id == 0) {
                        this->_hubNewDevice(registrationMsg, sender);
                    } else {
                        registrationMsg->receiver = 0;
                        registrationMsg->registrationType = 2;
                        this->_routeAndSend(registrationMsg);
                    }

                    break;
                }
                case 2: {   // route creation
                    // new device as a descendant node
                    if (!this->isRegistered()) return false;
                    this->tempRoutingTable[registrationMsg->tempID] = sender;
                    if (this->id == 0) {
                        this->_hubNewDevice(registrationMsg, sender);
                    } else {
                        this->_routeAndSend(registrationMsg);
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

                    if (this->tempRoutingTable.find(registrationMsg->tempID) == this->tempRoutingTable.end())
                        return false;

                    // TODO is temporally updating the routing table necessary?
                    // temporarily update the routing table to the new path
                    uint8_t originalNextHop = 0;
                    bool routeExists = this->routingTable.find(registrationMsg->newDeviceID) != this->routingTable.end();
                    if (routeExists) originalNextHop = this->routingTable[registrationMsg->newDeviceID];

                    this->_routeAndSend(registrationMsg, sender);

                    if (this->tempRoutingTable[registrationMsg->tempID] == 0) {
                        this->routingTable[registrationMsg->newDeviceID] = registrationMsg->newDeviceID;
                    } else {
                        this->routingTable[registrationMsg->newDeviceID] = this->tempRoutingTable[registrationMsg->tempID];
                    }
                    this->tempRoutingTable.erase(registrationMsg->tempID);

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
            if (!this->isRegistered()) return false;
            if (message->receiver != this->id) {
                this->_routeAndSend(message);
                return false;
            }
            auto *pingMsg = dynamic_cast<PingMessage *>(message);
            if (!pingMsg->isResponse) {
                pingMsg->isResponse = true;
                pingMsg->receiver = pingMsg->senderId;
                pingMsg->senderId = this->id;
                this->_routeAndSend(pingMsg);
                return false;
            }

            this->pings[pingMsg->pingId].responseTime = Timer::elapsed(pingMsg->timestamp, this->_getTime());


            break;
        }
        case 3: {   // add/remove to group message
            if (!this->isRegistered()) return false;
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
            if (!this->isRegistered()) return false;
            if (this->id == 1) {
                auto *errMsg = dynamic_cast<ErrorMessage *>(message);
                this->_printError(errMsg->errorCode, errMsg->erroneousMessage);
                return false;
            }

            _routeAndSend(message);
            break;
        }
        case 5: {   // disconnect message
            if (!this->isRegistered()) return false;
            auto *connectionMsg = dynamic_cast<ReDisconnectMessage *>(message);
            if (connectionMsg->receiver == this->id) {
                if (connectionMsg->isDisconnect) {
                    this->registered = false;
                }
                return false;
            }

            if (connectionMsg->isDisconnect) {
                this->routingTable.erase(connectionMsg->receiver);
                this->_routeAndSend(connectionMsg);
                return false;
            }

            // Reconnect message
            if (this->routingTable.count(connectionMsg->receiver) > 0) {
                // there is a path from this node to the reconnecting node, so deconstruct this path
                connectionMsg->isDisconnect = true;
            }
            // send the message on the old path, before updating the path
            this->_routeAndSend(connectionMsg);
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

void NetworkDevice::_hubNewDevice(RegistrationMessage *registrationMsg, uint8_t sender) {
    if (registrationMsg->newDeviceID == 0) {
        // if the device id is not set, assign first free ID
        RegistrationMessage answerMsg = RegistrationMessage(0, 0, registrationMsg->tempID, 3, true);
        for (uint8_t i = 0; true; ++i) {
            if (this->routingTable.count(i) < 1) {
                answerMsg.receiver = i;
                answerMsg.newDeviceID = i;
                answerMsg.extraField = true;
                break;
            }
            if (i == 255) break;
        }
        this->_routeAndSend(&answerMsg);
        this->routingTable[registrationMsg->newDeviceID] = sender;
        this->tempRoutingTable.erase(registrationMsg->tempID);
    } else if (this->routingTable.count(registrationMsg->newDeviceID) == 0) {
        RegistrationMessage answerMsg = RegistrationMessage(0, registrationMsg->newDeviceID, registrationMsg->tempID, 3, true);
        this->_routeAndSend(&answerMsg);
        this->routingTable[registrationMsg->newDeviceID] = sender;
        this->tempRoutingTable.erase(registrationMsg->tempID);
    } else {

        // if the device ID is set and there is a device with the ID already in the routing table,
        // the hub has to ping id and wait for timeout, then accept
        uint32_t time = this->_getTime();
        this->registrationPings.push_back({registrationMsg->newDeviceID, Timer(this->timeout).start(time), registrationMsg->tempID});
        PingMessage pingMsg = PingMessage(registrationMsg->newDeviceID, time % 256, this->id, false, time);
        this->_routeAndSend(&pingMsg);
    }
}

void NetworkDevice::registerDevice() {
    if (this->id == 0)
        throw std::bad_function_call();
    this->tempID = this->_getTime();

    auto msg = new RegistrationMessage(this->parent, this->id, this->tempID, 1);
    this->_routeAndSend(msg);
    delete msg;
}

bool NetworkDevice::isRegistered() {
    return this->registered;
}

bool NetworkDevice::update() {

    auto time = this->_getTime();

    // hub checks pending registration pings for timeouts
    // other devices do not add elements to the vector, so an if clause is not needed
    for (uint8_t i = 0; i < this->registrationPings.size(); ++i) {
        auto ping = this->registrationPings[i];
        if (!ping.timer.expired(time)) continue;

        this->registrationPings.erase(find(this->registrationPings.begin(), this->registrationPings.end(), ping));

        // The ping for an ID a device is trying to register with has timed out, so send a disconnect message on the old path
        ReDisconnectMessage msg = ReDisconnectMessage(ping.newDeviceID, true);
        this->_routeAndSend(&msg);
        // then update the routing table
        this->routingTable[ping.newDeviceID] = this->tempRoutingTable[ping.tempID];
        this->tempRoutingTable.erase(ping.tempID);
        // and accept the registration request
        RegistrationMessage answerMsg = RegistrationMessage(ping.newDeviceID, ping.newDeviceID, ping.tempID, 3, true);
        this->_routeAndSend(&answerMsg);

        // index is changed after erase, so decrement i
        --i;
    }

    if (!_messageAvailable()) return false;

    uint8_t* messageAddress[1];
    uint8_t sender = this->_read(messageAddress);
    Message* message = Message::fromRawBytes(*messageAddress);


    if (message->group) {
        // group messages are always broadcasted
        this->_routeAndSend(message, sender);
        // group message cannot contain messages that hops on the way need to process
        if (this->isInGroup(message->receiver)) {
            return this->_processMessage(message, sender);
        }
    } else {
        // forward data messages if this is not the receiver (including broadcasts)
        if (message->getType() == 0 && message->receiver != this->id) {
            this->_routeAndSend(message, sender);
            return false;
        }
        // message might contain info this device needs even if device is not the receiver
        return this->_processMessage(message, sender);
    }

    delete message;

    return false;
}

bool NetworkDevice::send(uint8_t receiver, uint8_t *data, uint16_t dataSize) {
    if (!this->isRegistered()) return false;
    return this->_assembleAndSend(receiver, false, data, dataSize);
}

bool NetworkDevice::sendToGroup(uint8_t group, uint8_t *data, uint16_t dataSize) {
    if (!this->isRegistered()) return false;
    return this->_assembleAndSend(group, true, data, dataSize);
}

uint16_t NetworkDevice::receive(uint8_t **data) {
    *data = this->lastData;
    return this->lastDataSize;
}

uint8_t NetworkDevice::ping(uint8_t targetID) {
    uint8_t pingID = this->_getMessageID();
    PingMessage pingMsg = PingMessage(targetID, pingID, this->id, false, this->_getTime());
    this->_routeAndSend(&pingMsg);
    return pingID;
}

uint32_t NetworkDevice::checkPing(uint8_t pingID)  {
    uint32_t responseTime = this->pings[pingID].responseTime;
    if (responseTime > 0) {
        this->pings.erase(pingID);
    }
    return responseTime;
}
