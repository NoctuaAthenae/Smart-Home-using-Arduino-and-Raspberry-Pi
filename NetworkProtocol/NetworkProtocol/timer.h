#ifndef NETWORKPROTOCOL_TIMER_H
#define NETWORKPROTOCOL_TIMER_H
#include <cstdint>


class Timer {
    /**
     * Time the timer started.
     */
    uint32_t startTime;

    /**
     * Duration of the timer
     */
    uint16_t duration;

    public:

    /**
     * Creates a new timer.
     * @param duration Duration of the timer.
     */
    Timer(uint16_t duration) : startTime(0), duration(duration) {}

    /**
     * Creates a new timer and starts it.
     * @param duration Duration of the timer.
     * @param currentTime The current time.
     */
    Timer(uint16_t duration, uint32_t currentTime) : startTime(currentTime), duration(duration) {}

    /**
     * Starts the timer.
     * @param time The current time.
     * @return This timer.
     */
    Timer start(uint32_t time);

    /**
     * Checks if the timer is expired.
     * @param time Current time.
     * @return True if the timer is expired.
     */
    bool expired(uint32_t time);

    /**
     * Calculates the elapsed time between the given times, while paying attention to overflows.
     * @param startingTime Starting time.
     * @param time Current time.
     * @return Elapsed time between the given times.
     */
    static uint32_t elapsed(uint32_t startingTime, uint32_t time);
};


#endif //NETWORKPROTOCOL_TIMER_H