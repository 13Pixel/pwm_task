/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "cli_app.h"

#include <esp_log.h>
#include <stdbool.h>
#include <stdio.h>

#include "cmd_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string_utils.h"
#include "task_attribute.h"
#include "uart_api.h"
#include "uart_command_handler.h"
/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
#define CLI_APP_RESPONSE_BUFFER_SIZE 150

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const char *TAG = "CLI_APP";
const char CLI_APP_bad_chars[] = "\r\n";

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
static char response_buffer[CLI_APP_RESPONSE_BUFFER_SIZE] = {0};
char *command_delimiter = "!";

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
static void CLI_APP_Message_Parser(void *pvParameters) {
    while (true) {
        char *message = (char *)malloc(UART_API_MAX_MESSAGE_SIZE);
        if (UartAPI_GetMessage(message) == false) {
            continue;
        }
        remchar(message, CLI_APP_bad_chars);
        if (message[0] == '\0') {
            ESP_LOGI(TAG, "command is empty");
            free(message);
            continue;
        }
        sCmd_Api_Launcher_Args launcher_args = {
            .command_lut = UART_CMD_Commands_lut,
            .command_lut_size = eUART_CMD_handler_Commands_Last,
            .command = message,
            .response_buffer = response_buffer,
            .response_buffer_size = CLI_APP_RESPONSE_BUFFER_SIZE,
            .delimiter = command_delimiter,
        };
        CMD_API_FindFunction(&launcher_args);
        ESP_LOGI(TAG, "message got %s", message);
        ESP_LOGI(TAG, "CLI response %s", response_buffer);
        free(message);
    }
    vTaskDelete(NULL);
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/
bool ClI_APP_Init() {
    UartAPI_Init();
    esp_log_level_set(TAG, ESP_LOG_INFO);
    xTaskCreatePinnedToCore(CLI_APP_Message_Parser, TaskAttributes_cli.name, 2048, NULL, TaskAttributes_cli.uxPriority, NULL, 0);

    return true;
}