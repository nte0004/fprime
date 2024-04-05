#include <Os/Queue.hpp>
#include <queue.h>

namespace Os {

    Queue::Queue() : m_handle(-1) {
    }

    QueueStatus Queue::createInternal(const Fw::StringBase &name, NATIVE_INT_TYPE depth, NATIVE_INT_TYPE msgSize) {
        // If queue already exist then try to correct course.
        if (this->m_handle != -1) {
            // Get the existing handle, resetting m_handle if it does not exist.
            QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
            
            if (handle == nullptr) {
                this->m_handle = -1;
                return QueueStatus::QUEUE_UNINITIALIZED;
            }

            // Try to delete the existing queue if depth or msgSize has changed.
            // May result in undefined behavior if task try to use the queue that was deleted.
            if (handle->uxLength != depth || handle->uxItemSize != msgSize) { 
                volatile NATIVE_UINT_TYPE msgsWaiting = uxQueueMessagesWaiting(handle);
               
                // Delete the existing queue if it is empty, if it isn't empty then fail.
                // TODO: Probably need a mutex here.
                if (msgsWaiting == 0) { 
                    vQueueDelete(handle);
                }
                else {
                    return QueueStatus::QUEUE_UNINITIALIZED;
                }
            }
            // Continue to use the existing queue since depth and msgSize have not changed.
            else {
                // Update the name if necessary:
                this->m_name = (this->m_name != name) ? name : this->m_name;
                
                return QueueStatus::QUEUE_OK;
            }                      
        }

        this->m_name = name;

        if (depth <= 0 || msgSize <= 0) {
            return QueueStatus::QUEUE_UNINITIALIZED;
        }

        // Create the queue.
        QueueHandle_t handle = xQueueCreate(depth, msgSize);
        
        if (handle == nullptr) {
            return QueueStatus::QUEUE_UNINITIALIZED;
        }

        this->m_handle = reinterpret_cast<POINTER_CAST>(handle);

        // If registry has been registered, register the queue.
        if (Queue::s_QueueRegistry) {
            Queue::s_QueueRegistry->regQueue(this);
        }

        Queue::s_numQueues++;

        return QueueStatus::QUEUE_OK;
    }

    Queue::~Queue() {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
        vQueueDelete(handle);
    }

    // Send serialized buffers
    QueueStatus Queue::send(const Fw::SerializeBufferBase &buffer, NATIVE_INT_TYPE priority, QueueBlocking block) {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
        TickType_t ticksToWait;
        
        if (handle == nullptr) {
            return QueueStatus::QUEUE_UNINITIALIZED;
        }

        
        if (buffer.getBuffLength() == 0) {
            return QueueStatus::QUEUE_EMPTY_BUFFER;
        }

        if (buffer.getBuffCapacity() > handle->uxItemSize) {
            return QueueStatus::QUEUE_SIZE_MISMATCH;
        }

        // Create pointer to buffer address
        const U8* itemToQueue = buffer.getBuffAddr();
        

        if (block == QUEUE_BLOCKING) {
            // The writing task will wait for space to become available on a full queue until:
            //  1. It is the highest priority task waiting OR; 
            //  2. It is equal priority with another task and has been waiting longer.
            ticksToWait = portMAX_DELAY;
        }
        else {  // block == QUEUE_NONBLOCKING
            ticksToWait = 0;
        }

        BaseType_t qStatus = xQueueSend(handle, itemToQueue, ticksToWait);   
        
        if (qStatus != pdPASS) { 
            return QueueStatus::QUEUE_FULL;
        }
        
        return QueueStatus::QUEUE_OK;

    }

    QueueStatus Queue::receive(Fw::SerializeBufferBase &buffer, NATIVE_INT_TYPE &priority, QueueBlocking block) {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
        TickType_t ticksToWait;
    
        if (handle == nullptr) {
            return QueueStatus::QUEUE_UNINITIALIZED;
        }

        if (buffer.getBuffCapacity() < handle->uxItemSize) {
            return QueueStatus::QUEUE_SIZE_MISMATCH;
        }

        // Create pointer to buffer address 
        U8* buffAddr = buffer.getBuffAddr();

        if (block == QUEUE_BLOCKING) {
            // Waits for data to become available on an empty queue in a similiar manner as send()
            ticksToWait = portMAX_DELAY;
        }
        else {
            ticksToWait = 0;
        }

        BaseType_t qStatus = xQueueReceive(handle, buffAddr, ticksToWait);
        
        if (qStatus != pdPASS) {
            return QueueStatus::QUEUE_NO_MORE_MSGS;
        }

        return QueueStatus::QUEUE_OK;
    }

    // Send raw buffers
    QueueStatus Queue::send(const U8* buffer, NATIVE_INT_TYPE size, NATIVE_INT_TYPE priority, QueueBlocking block) {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
        TickType_t ticksToWait;

        if (handle == nullptr) {
            return QueueStatus::QUEUE_UNINITIALIZED;
        }

        NATIVE_INT_TYPE maxItemSize = handle->uxItemSize;
        if (size > maxItemSize) {
            return QueueStatus::QUEUE_SIZE_MISMATCH;
        } 
        
        if (block == QUEUE_BLOCKING) {
            // The writing task will wait for space to become available on a full queue until:
            //  1. It is the highest priority task waiting OR; 
            //  2. It is equal priority with another task and has been waiting longer.
            ticksToWait = portMAX_DELAY;
        }
        else {  // block == QUEUE_NONBLOCKING
            ticksToWait = 0;
        }

        BaseType_t qStatus = xQueueSend(handle, buffer, ticksToWait);   
        
        if (qStatus != pdPASS) { 
            return QueueStatus::QUEUE_FULL;
        }
        
        return QueueStatus::QUEUE_OK;
    }
    
    QueueStatus Queue::receive(U8* buffer, NATIVE_INT_TYPE capacity, NATIVE_INT_TYPE &actualSize, NATIVE_INT_TYPE &priority,
                                QueueBlocking block) { 
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
        TickType_t ticksToWait;

        if (handle == nullptr) {
            return QueueStatus::QUEUE_UNINITIALIZED;
        }

        NATIVE_INT_TYPE itemSize = handle->uxItemSize;
        
        if (capacity < itemSize) {
            return QueueStatus::QUEUE_SIZE_MISMATCH;
        }

        if (block == QUEUE_BLOCKING) {
            // Waits for data to become available on an empty queue in a similiar manner as send()
            ticksToWait = portMAX_DELAY;
        }
        else {
            ticksToWait = 0;
        }

        BaseType_t qStatus = xQueueReceive(handle, buffer, ticksToWait);
        
        if (qStatus != pdPASS) {
            return QueueStatus::QUEUE_NO_MORE_MSGS;
        }

        return QueueStatus::QUEUE_OK;
    }

    NATIVE_INT_TYPE Queue::getNumMsgs() const {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);
        
        return static_cast<NATIVE_INT_TYPE>(uxQueueMessagesWaiting(handle));
        
    }    

    NATIVE_INT_TYPE Queue::getMaxMsgs() const {
        // FreeRTOS API does not support highwater mark for a queue.
        return -1; 
    }

    NATIVE_INT_TYPE Queue::getQueueSize() const {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);

        return static_cast<NATIVE_INT_TYPE>(handle->uxLength);
    }

    NATIVE_INT_TYPE Queue::getMsgSize() const {
        QueueHandle_t handle = reinterpret_cast<QueueHandle_t>(this->m_handle);

        return static_cast<NATIVE_INT_TYPE>(handle->uxItemSize);
    }    

}

