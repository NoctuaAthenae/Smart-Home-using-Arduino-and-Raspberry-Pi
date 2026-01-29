#include "ConnectionBenchmark.h"

bool ConnectionBenchmark::update(uint32_t time) {
    return this->timer.expired(time);
}

void ConnectionBenchmark::newAnswer(uint16_t rtt) {
    uint32_t previousRtt = this->averageRtt * this->receivedCount;
    this->averageRtt = (previousRtt + rtt) / ++this->receivedCount;
}