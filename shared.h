#include <esp/uart.h>
#include <stdio.h>
#include <espressif/esp_common.h>
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include "ssid_config.h"
#include "RF24/nRF24L01.h"
#include "RF24/RF24.h"
#include "bmp280/bmp280.h"
#include "i2c/i2c.h"

// Wemos
#define GPIO_LED 2

// I2C
#define BUS_I2C 0
#define SCL 14
#define SDA 12

// nrf24
#define CE_NRF 3
#define CS_NRF 0
#define CHANNEL 33

RF24 radio(CE_NRF, CS_NRF);
const uint8_t address[] = {0x01, 0x23, 0x45, 0x67, 0x89};

bmp280_t bmp280_dev;
typedef enum {
        BMP280_TEMPERATURE, BMP280_PRESSURE
} bmp280_quantity;

float read_bmp(bmp280_quantity quantity) {
	i2c_init(BUS_I2C, SCL, SDA, I2C_FREQ_100K);
        float temperature, pressure;

        // wait for measurement to complete
        while (bmp280_is_measuring(&bmp280_dev)) {};
        bmp280_read_float(&bmp280_dev, &temperature, &pressure, NULL);

        if (quantity == BMP280_TEMPERATURE) {
                return temperature;
        } else if (quantity == BMP280_PRESSURE) {
                return pressure;
        }

        return 0;
}
