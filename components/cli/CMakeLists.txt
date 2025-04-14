idf_component_register(SRCS "cli_app.c"
        INCLUDE_DIRS "." "include"
        PRIV_REQUIRES freertos attributes uart string_utils cmd )