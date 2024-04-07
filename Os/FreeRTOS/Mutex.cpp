#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Define Global
SemaphoreHandle_t xMutex;

void Task1(void *pvParameters)
{
    while (1)
    {
        // Take Mutex
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
           
            // Release Mutex
            xSemaphoreGive(xMutex);
        }

        // Delay if needed
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Task2(void *pvParameters)
{
    while (1)
    {
        // Take Mutex
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            

            // Release Mutex
            xSemaphoreGive(xMutex);
        }

        // Delay if needed
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void)
{
    // Create Mutex
    xMutex = xSemaphoreCreateMutex();

    // Create tasks
    xTaskCreate(Task1, "Task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(Task2, "Task2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    // FreeRTOS Scheduler
    vTaskStartScheduler();

    // if return something wrong
    while (1)
    {
    }

    return 0;
}
