idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_gpio
                       PRIV_REQUIRES spi_flash
		       PRIV_REQUIRES esp_timer)