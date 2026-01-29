#include "timer.h"

Timer Timer::start(uint32_t time) {
    this->startTime = time;
    return *this;
}

bool Timer::expired(uint32_t time) {
    return elapsed(this->startTime, time) > this->duration;
}

uint32_t Timer::elapsed(uint32_t startingTime, uint32_t time) {
    if (startingTime > time) {
        return UINT32_MAX - startingTime + time;
    }
    return time - startingTime;
}
