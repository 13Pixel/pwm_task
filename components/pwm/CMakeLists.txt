idf_component_register(SRCS "pwm_app.c" "pwm_api.c"
        INCLUDE_DIRS "." "include"
        PRIV_REQUIRES driver cmd attributes freertos uart esp_timer)
