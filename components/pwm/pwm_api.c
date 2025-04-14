/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "pwm_api.h"

#include <esp_log.h>
#include <stdbool.h>
#include <stdio.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "freertos/task.h"
#include "uart_api.h"
#include "uart_command_handler.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define PWM_API_MSG_QUEUE_OBJECTS 5
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const char *TAG = "PWM_API";
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
QueueHandle_t gPWMAPImessage_queue;
esp_timer_handle_t timer_handler;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
void PWM_API_Timer_callback(void *param)
{
    static bool ON;
    ON = !ON;
    gpio_set_level(GPIO_NUM_2, ON);
}
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
bool PWM_API_Start(sPWM_API_Set_PWM_Command_t *data)
{
    esp_timer_stop(timer_handler);
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 1000000 / (data->frequency)));

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
bool PWM_API_Init()
{
    gpio_set_direction(ePWM_API_pin_LED, GPIO_MODE_OUTPUT);

    const esp_timer_create_args_t my_timer_args = {
        .callback = &PWM_API_Timer_callback,
        .name = "My Timer"};
    ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));

    gPWMAPImessage_queue = xQueueCreate(PWM_API_MSG_QUEUE_OBJECTS, UART_API_MAX_MESSAGE_SIZE);

    if (gPWMAPImessage_queue == NULL)
    {
        ESP_LOGI(TAG, "Message queue creation failed");
    }

    return true;
}

bool PWM_API_GetMessage(sUART_Cmd_handler_Commands_t *command_args)
{
    if (xQueueReceive(gPWMAPImessage_queue, command_args, (TickType_t)portMAX_DELAY) != pdPASS)
    {
        return false;
    }
    return true;
}

bool PWM_API_AddMessage(sUART_Cmd_handler_Commands_t *command_args)
{
    if (xQueueSend(gPWMAPImessage_queue, command_args, (TickType_t)10) != pdPASS)
    {
        /* Failed to post the message, even after 10 ticks. */
        ESP_LOGI(TAG, "Message was not added to queue");
        return false;
    }
    return true;
}
