#include <FreeRTOS.h>
#include <semphr.h>
#include <Fw/Types/Assert.hpp>

namespace Os {

    class Mutex {
    private:
        SemaphoreHandle_t handle;

    public:
        Mutex() {
            // Create 
            handle = xSemaphoreCreateMutex();
            FW_ASSERT(handle != nullptr, "Failed to create mutex");
        }

        ~Mutex() {
            vSemaphoreDelete(handle);
        }

        void lock() {
            // Lock
            if (xSemaphoreTake(handle, portMAX_DELAY) != pdTRUE) {
                FW_ASSERT(false, "Failed to acquire mutex");
            }
        }

        void unlock() {
            // Unlock
            if (xSemaphoreGive(handle) != pdTRUE) {
                FW_ASSERT(false, "Failed to release mutex");
            }
        }

        // try_lock
        bool try_lock() {
            return xSemaphoreTake(handle, 0) == pdTRUE;
        }
    };

} 
