/*
	Make an access point to which we can connect.
	It dynamically allocates an IP address to anyone connecting.
	Receive data from nrf24 transmitter and send it forward.
*/

#include "shared.h"
#include <dhcpserver.h>
static float rx_data;

static void set_ap(void) {
	
	sdk_wifi_set_opmode(SOFTAP_MODE);

	struct ip_info info;
	IP4_ADDR(&info.ip, 192, 168, 0, 1);
	IP4_ADDR(&info.gw, 192, 168, 0, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	sdk_wifi_set_ip_info(1, &info);
	
	struct sdk_softap_config config = { 
		WIFI_SSID, 
		WIFI_PASS, 
		strlen(WIFI_SSID), 
		3, 
		AUTH_WPA_WPA2_PSK, 
		0, 3, 100};

	sdk_wifi_softap_set_config(&config);
	
	ip_addr_t first_client_ip;
	IP4_ADDR(&first_client_ip, 192, 168, 0, 2);
	dhcpserver_start(&first_client_ip, 4);
	dhcpserver_set_dns(&info.ip);
	dhcpserver_set_router(&info.ip);

	printf("Wifi ssid: %s\n", config.ssid);
}

void receive_data_task(void *pvParameters) {
        gpio_enable(CS_NRF, GPIO_INPUT);

        radio.begin();
        radio.setChannel(CHANNEL);
        radio.openReadingPipe(1, address);
        radio.startListening();
	
	while (1) {
		if (radio.available()) {
			radio.read(&rx_data, sizeof(float));
			printf("Received message: %f\n", rx_data);

			gpio_write(GPIO_LED, 0);
		}
		radio.powerDown();
		vTaskDelay(pdMS_TO_TICKS(50));
		gpio_write(GPIO_LED, 1);
		vTaskDelay(pdMS_TO_TICKS(50));
		radio.powerUp();
	}
}

static void bmp_task(void *pvParameters)  {
       // BMP280 configuration
        bmp280_params_t params;
        bmp280_init_default_params(&params);
        bmp280_dev.i2c_dev.bus = BUS_I2C;
        bmp280_dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0;
        bmp280_init(&bmp280_dev, &params);

        float temperature, pressure;

        while (1) {

                vTaskDelay(pdMS_TO_TICKS(5000));
                temperature = read_bmp(BMP280_TEMPERATURE);
                pressure = read_bmp(BMP280_PRESSURE);

                printf("Temperature on receiver: %f °C\n", temperature);
                printf("Pressure on receiver: %f mBar\n", 0.01*pressure);
        }	

}

extern "C" void user_init(void) {
	uart_set_baud(0, 115200);
	
	set_ap();

	i2c_init(BUS_I2C, SCL, SDA, I2C_FREQ_100K);
	gpio_enable(SCL, GPIO_OUTPUT);
	gpio_enable(GPIO_LED, GPIO_OUTPUT);
	gpio_write(GPIO_LED, 1);

	xTaskCreate(&bmp_task, "Measure data", 512, NULL, 2, NULL);
	xTaskCreate(receive_data_task, "Listen to radio for incoming data", 512, NULL, 2, NULL);	
}

