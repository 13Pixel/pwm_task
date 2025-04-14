idf_component_register(SRCS "uart_command_handler.c" "uart_driver.c" "uart_api.c"
        INCLUDE_DIRS "." "include"
        PRIV_REQUIRES freertos cmd pwm driver string_utils attributes)