#include <Os/Task.hpp>
#include <Fw/Types/Assert.hpp>
#include <Fw/Logger/Logger.hpp>
#include <cstring>
#include <task.h>
#include <FreeRTOSConfig.h>
#include <projdefs.h>

namespace Os {

    void Task::validate_arguments(taskRoutine routine, Fw::StringBase& name, NATIVE_UINT_TYPE& stackSizeBytes,
                                                NATIVE_UINT_TYPE& priority) {        
        const NATIVE_UINT_TYPE minPriority = 0;
        const NATIVE_UINT_TYPE maxPriority = FreeRTOSConfig::configMAX_PRIORITIES;
        const NATIVE_UINT_TYPE maxNameLen = FreeRTOSConfig::configMAX_TASK_NAME_LEN;
        const NATIVE_UINT_TYPE minStackSize = FreeRTOSConfig::configMINIMAL_STACK_SIZE;
        
        // Ensure routine not null.
        FW_ASSERT(routine);
 
        // Ensure name is not too large.
        const char* ogName = name.toChar();
        const NATIVE_INT_TYPE ogNameLen = std::strlen(ogName);
        if (ogNameLen > maxNameLen) {
            Fw::Logger::logMsg("[WARNING] Large name length of %d chars being clamped to %d chars\n", ogNameLen, maxNameLen);
            
            // Truncate name via inserting a null byte. 
            ogName[maxNameLen] = '\0';
        }

        if (maxPriority < 0) {
            Fw::Logger::logMsg("[WARNING] Unable to determine max priority with error %s. Discarding priority.\n",
                                reinterpret_cast<POINTER_CAST>(strerror(errno)));
            priority = Os::Task::TASK_DEFAULT;
        }

        if (priority != Task::TASK_DEFAULT and priority < static_cast<Task::ParamType>(minPriority)) {
            Fw::Logger::logMsg("[WARNING] Low task priority of %d being clamped to %d\n", priority, minPriority);
            priority = minPriority;
        }

        if (priority != Task::TASK_DEFAULT and priority > static_cast<Task::ParamType>(maxPriority)) {
            Fw::Logger::logMsg("[WARNING] High task priority of %d being clamped to %d\n", priority, maxPriority);
            priority = maxPriority;
        }

        // OSAL expects stackSize to be bytes, FreeRTOS expects words, so we convert.
        const NATIVE_INT_TYPE stackSizeWords = (stackSizeBytes + (sizeof(StackType_t) - 1)) / sizeof(StackType_t);
        stackSizeBytes = stackSizeWords;

        if (stackSizeWords != Task::TASK_DEFAULT and stackSizeWords < minStackSize) {
            Fw::Logger::logMsg("[WARNING] Stack size %d (words) too small, setting to minimum of %d\n", stackSizeWords, minStackSize);
            stackSizeBytes = minStackSize;
        }

    }

    Task::Task() : m_handle(reinterpret_cast<POINTER_CAST>(nullptr)), m_identifier(0), m_started(false), m_suspendedOnPurpose(false),
                            m_routineWrapper() {
    }

    Task::TaskStatus Task::start(Fw::StringBase& name, taskRoutine routine, void* arg, ParamType priority, ParamType stackSize,
                                 ParamType cpuAffinity,      //cpuAffinity not used by FreeRTOS, but expected by OSAL.
                                 ParamType identifier) {
        // Check and correct potential issues with the arguments. 
        validate_arguments(routine, name, stackSize, priority);
        
        // xTaskCreate expects a const char* const name.
        const char* const namePtr = name.toChar();

        this->m_name = "TP_";
        this->m_name += name;
        this->m_identifier = identifier;
        this->m_routineWrapper.routine = routine;
        this->m_routineWrapper.arg = arg;
        
        // Task are referenced by the handle.
        TaskHandle_t* handle;

        // Create the task
        BaseType_t status = xTaskCreate(routine, namePtr, arg, priority, handle);
        if (status != pdPass) {
            return TaskStatus::TASK_ERROR_RESOURCES;
        }
        
        this->m_handle = reinterpret_cast<POINTER_CAST>(&handle);
        Task::s_numTasks++;
        
        // If a registry has been registered, register task.
        if (Task::s_taskRegistry) {
            Task::s_taskRegistry->addTask(this);
        }
        
        return TaskStatus::TASK_OK;

    }

    Task::TaskStatus Task::delay(NATIVE_UINT_TYPE milliseconds) {
        // Number of ticks will be the quotient, the remainder is discarded. 
        const NATIVE_UINT_TYPE ticks = milliseconds / portTICK_PERIOD_MS;
        
        vTaskDelay(ticks);
        return TaskStatus::TASK_OK;
    }

    TaskHandle_t Task::getTaskHandle() {
        POINTER_CAST rawHandle = this->getRawHandle();
        TaskHandle_t taskHandle = *reinterpret_cast<TaskHandle_t*>(rawHandle);
        
        return taskHandle;
    }

    void Task::suspend(bool onPurpose) {
        TaskHandle_t taskHandle = getTaskHandle();
        
        vTaskSuspend(taskHandle);
        m_suspendedOnPurpose = onPurpose;
    }

    void Task::resume() {
        TaskHandle_t taskHandle = getTaskHandle();
        
        vTaskResume(taskHandle);
    }

    bool Task::isSuspended() {
        eTaskState state;
        TaskHandle_t taskHandle = getTaskHandle();
        
        state = eTaskGetState(taskHandle);
        return (state == eSuspended);
    }
}



