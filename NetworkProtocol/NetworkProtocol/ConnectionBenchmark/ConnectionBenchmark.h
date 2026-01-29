#ifndef NETWORKPROTOCOL_CONNECTIONBENCHMARK_H
#define NETWORKPROTOCOL_CONNECTIONBENCHMARK_H
#include <cstdint>
#include "../timer.h"


class ConnectionBenchmark {
    /**
     * Number of pings returned.
     */
    uint8_t receivedCount;

    /**
     * Average round trip time of the received pings.
     */
    uint16_t averageRtt;

    /**
     * Timer for the Benchmark.
     */
    Timer timer;

public:
    /**
     * Sets up the benchmark for one device.
     * @param timeout Time maximum waited beginning at the current time.
     * @param currentTime Current time.
     */
    explicit ConnectionBenchmark(uint16_t timeout, uint32_t currentTime) : receivedCount(0), averageRtt(0), timer(timeout, currentTime) {}

    /**
     * Checks if the benchmark is finished.
     * @return True, if benchmark is finished.
     */
    bool update(uint32_t time);

    /**
     * Got a new answer, save it.
     * @param rtt Round trip time of the ping.
     */
    void newAnswer(uint16_t rtt);
};


#endif //NETWORKPROTOCOL_CONNECTIONBENCHMARK_H