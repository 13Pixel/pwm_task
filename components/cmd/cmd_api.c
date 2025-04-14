/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include "cmd_api.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
static const char *TAG = "CMD_API";

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
bool CMD_API_FindFunction(sCmd_Api_Launcher_Args *launcher_args) {
    if (launcher_args->command == NULL) {
        ESP_LOGI(TAG, "Message not provided");
        return false;
    }
    if (launcher_args->command_lut == NULL) {
        ESP_LOGI(TAG, " LUT not provided");
        return false;
    }
    sCMD_API_Args handler_args = {
        .response_buffer = launcher_args->response_buffer,
        .response_buffer_size = launcher_args->response_buffer_size,
    };
    bool is_valid = false;
    uint8_t command_id = 0;
    int len = strlen(launcher_args->command);
    char *command_name = &launcher_args->command[len - 2];
    for (uint8_t command = 0; command < launcher_args->command_lut_size; command++) {
        // handler_args.arguments = command_parameters;

        if (strcmp(launcher_args->command_lut[command].name, command_name) == 0) {
            handler_args.arguments = strndup(launcher_args->command, len - 2);
            command_id = command;
            is_valid = true;
            break;
        }
    }
    if (is_valid == false) {
        snprintf(launcher_args->response_buffer, launcher_args->response_buffer_size, "Function is not found(%s)\n", command_name);
        return false;
    }
    if (launcher_args->command_lut[command_id].function_pointer((void *)&handler_args) == false) {
        snprintf(launcher_args->response_buffer, launcher_args->response_buffer_size, "Function arguments is wrong\n");
        return false;
    }
    return true;
}
