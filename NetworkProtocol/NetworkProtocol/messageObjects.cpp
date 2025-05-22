#include "messageObjects.h"
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos,set) ((var) | (set<<(pos)))

bool Message::isGroup() {
    return CHECK_BIT(this->typeAndGroups, 0);
}

bool Message::isGroupAscending() {
    return CHECK_BIT(this->typeAndGroups, 1);
}

void Message::setGroup(bool set) {
    this->typeAndGroups = SET_BIT(this->typeAndGroups, 0, set);
}

void Message::setGroupAscending(bool set) {
    this->typeAndGroups = SET_BIT(this->typeAndGroups, 1, set);
}

void Message::addChecksum() {
    // TODO implement checksum
}
