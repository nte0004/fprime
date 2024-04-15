#include "FreeRTOS.h"
#include "semphr.h"

class Mutex {
private:
    SemaphoreHandle_t mutex; // FreeRTOS mutex variable

public:
    Mutex() {
        //Create
        mutex = xSemaphoreCreateMutex();
    }

    ~Mutex() {
        //Delete
        vSemaphoreDelete(mutex);
    }

    void lock() {
        //lock
        xSemaphoreTake(mutex, portMAX_DELAY);
    }

    void unlock() {
        //unlock
        xSemaphoreGive(mutex);
    }

    //
    bool try_lock() {
        return xSemaphoreTake(mutex, 0) == pdPASS;
    }
};
