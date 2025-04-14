/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "uart_api.h"

#include <driver/uart.h>
#include <esp_log.h>
#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "task_attribute.h"
#include "uart_driver.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define MUTEX_WAIT 10
#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (1)
#define UART_API_MSG_QUEUE_OBJECTS 3
#define UART_API_DELIMITER '!'
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const char* TAG = "UART_API";

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static QueueHandle_t g_uart_queue;
QueueHandle_t gUartAPI_message_queue;
SemaphoreHandle_t g_mutex = NULL;
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void UartAPI_Read_Buffer(void* pvParameters) {
    uart_event_t event;
    size_t buffered_size;
    char* dtmp = (char*)malloc(UART_API_MAX_MESSAGE_SIZE);

    for (;;) {
        // Waiting for UART event.
        if (xQueueReceive(g_uart_queue, (void*)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, UART_API_MAX_MESSAGE_SIZE);
            ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch (event.type) {
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(g_uart_queue);
                    break;
                    // Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(g_uart_queue);
                    break;
                    // UART_PATTERN_DET
                case UART_PATTERN_DET:
                    uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                    int pos = uart_pattern_pop_pos(EX_UART_NUM);
                    ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                    if (pos == -1) {
                        // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                        // record the position. We should set a larger queue size.
                        // As an example, we directly flush the rx buffer here.
                        uart_flush_input(EX_UART_NUM);
                    } else {
                        uart_read_bytes(EX_UART_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
                        uint8_t pat[PATTERN_CHR_NUM + 1];
                        memset(pat, 0, sizeof(pat));
                        uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                        ESP_LOGI(TAG, "read data: %s", dtmp);
                        ESP_LOGI(TAG, "delimiter found");
                        if (xQueueSend(gUartAPI_message_queue,
                                       dtmp,
                                       (TickType_t)100) != pdPASS) {
                            /* Failed to post the message, even after 10 ticks. */
                            ESP_LOGI(TAG, "Message was not added to queue");
                        }
                    }
                    break;
                    // Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool UartAPI_GetMessage(char* message) {
    if (xQueueReceive(gUartAPI_message_queue,
                      message,
                      (TickType_t)portMAX_DELAY) != pdPASS) {
        /* *pxRxedPointer now points to xMessage. */
        return false;
    }

    return true;
}

bool UartAPI_SendString(char* message) {
    return true;
}

bool UartAPI_Init() {
    esp_log_level_set(TAG, ESP_LOG_INFO);

    uart_driver_install(EX_UART_NUM, UART_API_MAX_MESSAGE_SIZE * 2, UART_API_MAX_MESSAGE_SIZE * 2, 20, &g_uart_queue, 0);

    UartDriver_Init(EX_UART_NUM);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    // Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, UART_API_DELIMITER, PATTERN_CHR_NUM, 9, 0, 0);
    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 3);
    g_mutex = xSemaphoreCreateMutex();
    if (g_mutex == NULL) {
        return false;
    }
    // Create message queue
    gUartAPI_message_queue = xQueueCreate(UART_API_MSG_QUEUE_OBJECTS, UART_API_MAX_MESSAGE_SIZE);

    if (gUartAPI_message_queue == NULL) {
        ESP_LOGI(TAG, "Message queue creation failed");
    }
    // Create a task to handler UART event from ISR
    xTaskCreatePinnedToCore(UartAPI_Read_Buffer, TaskAttributes_uart.name, 2048, NULL, TaskAttributes_uart.uxPriority, NULL, 0);
    return true;
}
