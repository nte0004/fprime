#include <task.h>
#include <Os/InterruptLock.hpp>

namespace Os {
    
    void InterruptLock::lock() {
        taskENTER_CRITICAL();
     }

    void InterruptLock::unlock() {
        taskEXIT_CRITICAL();
     }

}
