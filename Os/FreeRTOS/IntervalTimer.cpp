/**
 * FreeRTOS/IntervalTimer.cpp:
 *
 * 
 */
#include <Os/IntervalTimer.hpp>
#include <Fw/Types/Assert.hpp>
#include <FreeRTOS.h>
#include <task.h>


namespace Os {
    void IntervalTimer::getRawTime(RawTime& time) {
        tick_count = xTaskGetTickCount();

        // Support for Arm Cortex-M ports.
        #ifdef configSYSTICK_CLOCK_HZ
            // Store the current raw time in seconds.
            time.upper = tick_count / configSYSTICK_CLOCK_HZ;

            // Store the current raw time in nanoseconds.
            time.lower = (tick_count % configSYSTICK_CLOCK_HZ) * (1000000000 / configSYSTICK_CLOCK_HZ);

        // Everything else.
        #else
            // Store the current raw time in seconds.
            time.upper = tick_count / configTICK_RATE_HZ;

            // Store the current raw time in nanoseconds.
            time.lower = (tick_count % configTICK_RATE_HZ) * (1000000000 / configTICK_RATE_HZ);
        #endif
    }

    U32 IntervalTimer::getDiffUsec(const RawTime& t1In, const RawTime& t2In) {
        RawTime result = {0, 0};

        // Calculate the difference in seconds and nanoseconds
        if (t1In.lower < t2In.lower) {
            result.upper = t1In.upper - t2In.upper - 1; // subtract nsec carry to seconds
            result.lower = t1In.lower + (1000000000 - t2In.lower);
        } else {
            result.upper = t1In.upper - t2In.upper;
            result.lower = t1In.lower - t2In.lower;
        }

        // Convert the result to microseconds
        return (result.upper * 1000000) + (result.lower / 1000);
    }
}