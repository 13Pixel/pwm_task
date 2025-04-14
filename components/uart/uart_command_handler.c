/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "uart_command_handler.h"

#include <esp_log.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "cmd_api.h"
#include "pwm_api.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const char *TAG = "UART_CMD_HANDLER";

const CMD_API_Commmands_t UART_CMD_Commands_lut[] = {
    [eUART_CMD_handler_Commands_SetPWM] = {
        .function_pointer = &UART_CMD_Handler_SetPWM,
        .name = "Hz",
    },
};
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
bool UART_CMD_Handler_SetPWM(sCMD_API_Args *args)
{
    if (args == NULL)
    {
        ESP_LOGI(TAG, "Wrong args provided");
        return false;
    }
    if (args->arguments == NULL)
    {
        ESP_LOGI(TAG, "Wrong args provided");
        return false;
    }
    bool is_kHz = false;
    if (strlen(args->arguments) > 0 && args->arguments[strlen(args->arguments) - 1] == 'k')
    {
        args->arguments[strlen(args->arguments) - 1] = '\0';
        is_kHz = true;
    }

    void *ptr = memchr(args->arguments, ',', strlen(args->arguments));
    if (ptr != NULL)
    {
        *(char *)ptr = '.'; // cast and replace
    }
    char *endptr;
    double value = strtod(strndup(args->arguments, strlen(args->arguments)), &endptr);
    if (*endptr != '\0')
    {
        ESP_LOGI(TAG, "Wrong args provided");
        return false;
    }

    if (is_kHz == true)
    {
        value *= 1000;
    }
    int frequency = (int)value;

    if (frequency < 1 || frequency > 10000)
    {
        ESP_LOGI(TAG, "Wrong args provided");
        return false;
    }
    sPWM_API_Set_PWM_Command_t *local_data = calloc(1, sizeof(sPWM_API_Set_PWM_Command_t));

    local_data->frequency = frequency;
    local_data->pin = ePWM_API_pin_LED;

    sUART_Cmd_handler_Commands_t command = {
        .command_enum = eUART_CMD_handler_Commands_SetPWM,
        .data = local_data,
    };
    snprintf(args->response_buffer, args->response_buffer_size, "Command Handler: correct\n");
    PWM_API_AddMessage(&command);
    return true;
}