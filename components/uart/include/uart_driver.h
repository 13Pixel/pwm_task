#ifndef __MAIN_UART_DRIVER_H__
#define __MAIN_UART_DRIVER_H__

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/
typedef enum eUartDriver_Baud_Rate_Enum_t {
    eUartDriver_BaudRate_First = 0,
    eUartDriver_BaudRate_9600 = eUartDriver_BaudRate_First,
    eUartDriver_BaudRate_14400,
    eUartDriver_BaudRate_19200,
    eUartDriver_BaudRate_28800,
    eUartDriver_BaudRate_33600,
    eUartDriver_BaudRate_38400,
    eUartDriver_BaudRate_57600,
    eUartDriver_BaudRate_115200,
    eUartDriver_BaudRate_default = eUartDriver_BaudRate_115200,
    eUartDriver_BaudRate_128000,
    eUartDriver_BaudRate_230400,
    eUartDriver_BaudRate_921600,
    eUartDriver_BaudRate_Last
} eUartDriver_Baud_Rate_Enum_t;
/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/
bool UartDriver_Init(uint8_t uart_num);

#endif  // __MAIN_UART_DRIVER_H__
