#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "sht31.h"
#include "queue.h"
#include "semphr.h"

extern UART_HandleTypeDef huart2;

/* Task prototypes */
void timerWakeTask(void *argument);
void sensorTask(void *argument);
void extiTask(void *argument);
void uartTask(void *argument);

/* RTOS objects */
QueueHandle_t uartQueue;
SemaphoreHandle_t extiSemaphore;

TaskHandle_t timerTaskHandle;
TaskHandle_t sensorTaskHandle;
TaskHandle_t extiTaskHandle;
TaskHandle_t uartTaskHandle;

/* FreeRTOS init and idle hook */
void MX_FREERTOS_Init(void);
void vApplicationIdleHook(void);

/* Idle hook */
void vApplicationIdleHook(void) {}

/* Create RTOS objects: queue, semaphore, tasks */
void MX_FREERTOS_Init(void)
{
    /* Queue for UART messages (char* pointers) */
    uartQueue = xQueueCreate(10, sizeof(char*));

    /* Binary semaphore for EXTI events */
    extiSemaphore = xSemaphoreCreateBinary();

    /* Timer wake task: waits for notification from TIM3 ISR */
    xTaskCreate(timerWakeTask, "timer", 256, NULL, 5, &timerTaskHandle);

    /* Sensor task: reads SHT31 and sends formatted string to UART queue */
    xTaskCreate(sensorTask, "sensor", 256, NULL, 4, &sensorTaskHandle);

    /* EXTI task: reacts to button press and sends message to UART queue */
    xTaskCreate(extiTask, "exti", 256, NULL, 3, &extiTaskHandle);

    /* UART task: serialises all messages over USART2 */
    xTaskCreate(uartTask, "uart", 256, NULL, 2, &uartTaskHandle);
}

/**
 * @brief  Timer wake task.
 *         Wakes on task notification from TIM3 IRQ, pulses PA6,
 *         then notifies sensorTask to perform a measurement.
 */
void timerWakeTask(void *argument)
{
    for (;;)
    {
        /* Wait indefinitely for notification from ISR */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

        /* Trigger sensor task */
        xTaskNotifyGive(sensorTaskHandle);
    }
}

/**
 * @brief  Sensor task.
 *         Waits for notification, reads SHT31, formats message,
 *         and sends pointer to UART queue.
 */
void sensorTask(void *argument)
{
    static char msg[64];
    char *pMsg = msg;

    for (;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

        int temp, hum;

        if (sht31_read(&temp, &hum) == HAL_OK)
        {
            sprintf(msg, "T=%d H=%d\r\n", temp, hum);
        }
        else
        {
            sprintf(msg, "SHT31 error\r\n");
        }

        /* Send pointer to message buffer to UART task */
        xQueueSend(uartQueue, &pMsg, 0);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
    }
}

/**
 * @brief  EXTI task.
 *         Waits on binary semaphore given from EXTI ISR,
 *         pulses PA8 and sends "Button pressed!" to UART queue.
 */
void extiTask(void *argument)
{
    static char msg[] = "Button pressed!\r\n";
    char *pMsg = msg;

    for (;;)
    {
        xSemaphoreTake(extiSemaphore, portMAX_DELAY);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

        xQueueSend(uartQueue, &pMsg, 0);

        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    }
}

/**
 * @brief  UART task.
 *         Receives message pointers from queue and transmits via USART2.
 */
void uartTask(void *argument)
{
    char *pMsg;

    for (;;)
    {
        if (xQueueReceive(uartQueue, &pMsg, portMAX_DELAY) == pdTRUE)
        {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);

            HAL_UART_Transmit(&huart2,
                              (uint8_t*)pMsg,
                              strlen(pMsg),
                              HAL_MAX_DELAY);

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
        }
    }
}

/**
 * @brief  Pre sleep processing for FreeRTOS tickless idle.
 *         Called before the MCU enters sleep.
 *         Suspends HAL tick and executes WFI.
 */
void PreSleepProcessing(uint32_t expectedIdleTime)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);

    HAL_SuspendTick();   // Stop HAL SysTick
    __WFI();             // Wait for interrupt (sleep)
}

/**
 * @brief  Post sleep processing for FreeRTOS tickless idle.
 *         Resumes HAL tick and clears PA10.
 */
void PostSleepProcessing(uint32_t expectedIdleTime)
{
    HAL_ResumeTick();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
}
