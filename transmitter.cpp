// Connect to an AP and send data captured by BMP280

#include "shared.h"

static void wifi_connect(void) {

        sdk_wifi_set_opmode(STATION_MODE);

        sdk_wifi_station_set_auto_connect(1);

        struct sdk_station_config config = { WIFI_SSID, WIFI_PASS };
        sdk_wifi_station_set_config(&config);
        sdk_wifi_station_connect();	
}

static void bmp_task(void *pvParameters) {

       // BMP280 configuration
        bmp280_params_t params;
        bmp280_init_default_params(&params);
	params.mode = BMP280_MODE_FORCED;	
        bmp280_dev.i2c_dev.bus = BUS_I2C;
        bmp280_dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0;
        bmp280_init(&bmp280_dev, &params);

	float temperature, pressure;	
	gpio_enable(GPIO_LED, GPIO_OUTPUT);
	
	while (1) {

		temperature = read_bmp(BMP280_TEMPERATURE);
		pressure = read_bmp(BMP280_PRESSURE);

		printf("Temperature on transmitter: %f °C\n", temperature);
		printf("Pressure on transmitter: %f mBar\n", 0.01*pressure);		

		gpio_write(GPIO_LED, 0);
		radio.powerUp();
		radio.stopListening();
		radio.write(&temperature, sizeof(temperature));
		radio.write(&pressure, sizeof(pressure));
		radio.powerDown();

		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_write(GPIO_LED, 1);	
		vTaskDelay(pdMS_TO_TICKS(10000));
	}
}

extern "C" void user_init(void)
{
	uart_set_baud(0, 115200);
	i2c_init(BUS_I2C, SCL, SDA, I2C_FREQ_100K);
	gpio_enable(SCL, GPIO_OUTPUT);
	gpio_enable(CS_NRF, GPIO_OUTPUT);

	wifi_connect();	

	radio.begin();
	radio.setChannel(CHANNEL);
	radio.openWritingPipe(address);
	radio.powerDown();

	xTaskCreate(&bmp_task, "BMP task", 256, NULL, 2, NULL);
}
