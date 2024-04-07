#include <Os/WatchdogTimer.hpp>
#include <FreeRTOS.h>
#include <timers.h>

namespace Os {

    // Constructor
    WatchdogTimer::WatchdogTimer() {
        // Initialize the watchdog timer hardware or driver if needed

        // Create the FreeRTOS timer without starting it
        m_timerHandle = xTimerCreate("WatchdogTimer", // Name of the timer
                                     pdMS_TO_TICKS(100), // Default period (not used)
                                     pdFALSE, // Auto-reload
                                     nullptr, // Timer ID (not used)
                                     timerCallback); // Timer callback function
        // Check if the timer creation was successful
        if (m_timerHandle == nullptr) {
            // Handle error
            // Example: throw std::runtime_error("Timer creation failed");
        }
    }

    // Destructor
    WatchdogTimer::~WatchdogTimer() {
        // Delete the FreeRTOS timer if it exists
        if (m_timerHandle != nullptr) {
            xTimerDelete(m_timerHandle, portMAX_DELAY);
        }
    }

    // Start the watchdog timer with a timeout in ticks
    WatchdogTimer::WatchdogStatus WatchdogTimer::startTicks(I32 delayInTicks, WatchdogCb p_callback, void* parameter) {
        // Start the FreeRTOS timer with the specified timeout in ticks
        if (m_timerHandle != nullptr) {
            // Save callback and parameter
            m_cb = p_callback;
            m_parameter = parameter;
            // Start the timer
            if (xTimerChangePeriod(m_timerHandle, pdMS_TO_TICKS(delayInTicks), portMAX_DELAY) == pdPASS &&
                xTimerStart(m_timerHandle, portMAX_DELAY) == pdPASS) {
                return WATCHDOG_OK;
            }
        }
        return WATCHDOG_START_ERROR;
    }

    // Start the watchdog timer with a timeout in milliseconds
    WatchdogTimer::WatchdogStatus WatchdogTimer::startMs(I32 delayInMs, WatchdogCb p_callback, void* parameter) {
        // Convert milliseconds to ticks and call startTicks
        return startTicks(delayInMs, p_callback, parameter);
    }

    // Restart the watchdog timer
    WatchdogTimer::WatchdogStatus WatchdogTimer::restart() {
        // Restart the FreeRTOS timer
        if (m_timerHandle != nullptr && xTimerReset(m_timerHandle, portMAX_DELAY) == pdPASS) {
            return WATCHDOG_OK;
        }
        return WATCHDOG_CANCEL_ERROR;
    }

    // Cancel (stop) the watchdog timer
    WatchdogTimer::WatchdogStatus WatchdogTimer::cancel() {
        // Stop the FreeRTOS timer
        if (m_timerHandle != nullptr && xTimerStop(m_timerHandle, portMAX_DELAY) == pdPASS) {
            return WATCHDOG_OK;
        }
        return WATCHDOG_CANCEL_ERROR;
    }

    // Timer callback function
    void WatchdogTimer::timerCallback(TimerHandle_t xTimer) {
        // Retrieve the WatchdogTimer instance associated with the timer
        WatchdogTimer* watchdog = static_cast<WatchdogTimer*>(pvTimerGetTimerID(xTimer));
        // Call the callback function with its parameter
        if (watchdog != nullptr && watchdog->m_cb != nullptr) {
            watchdog->m_cb(watchdog->m_parameter);
        }
    }

    // Static timer callback wrapper to call member function
    void WatchdogTimer::timerCallbackWrapper(TimerHandle_t xTimer) {
        // Retrieve the WatchdogTimer instance associated with the timer
        WatchdogTimer* watchdog = static_cast<WatchdogTimer*>(pvTimerGetTimerID(xTimer));
        // Call the member function timerCallback
        if (watchdog != nullptr) {
            watchdog->timerCallback(xTimer);
        }
    }
}