/*
	Make an access point to which we can connect.
	It dynamically allocates an IP address to anyone connecting.
	Receive data from nrf24 transmitter and send it forward.
*/

#include "shared.h"
#include <dhcpserver.h>

float rx_data;
float temperature_receiver = 0;
float temperature_transmitter = 0;
float pressure_receiver = 0;
float pressure_transmitter = 0;
char api_string[100];
int second = 1000000;

// Parse url to be usable for the api
static void parse_url(float temperature, float pressure, int station) {

	char temp_string[10], press_string[10];
	sprintf(temp_string, "%.2f", temperature);
	sprintf(press_string, "%.2f", pressure);


	if (station == 0)
		sprintf(api_string, "%s%s%s%s%s%s",  AT_CHTTPSEND, "v0=", temp_string, "&v1=", press_string, "\"");
	else
		sprintf(api_string, "%s%s%s%s%s%s", AT_CHTTPSEND, "v2=", temp_string, "&v3=", press_string, "\"");
}

// Set access point
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

			if (rx_data > 800)
				pressure_transmitter = rx_data;	
			else
				temperature_transmitter = rx_data;
	
			parse_url(temperature_transmitter, pressure_transmitter, 1);	
			softuart_puts(0, api_string);			
		}
	}
}

static void bmp_task(void *pvParameters)  {
       // BMP280 configuration
        bmp280_params_t params;
        bmp280_init_default_params(&params);
        bmp280_dev.i2c_dev.bus = BUS_I2C;
        bmp280_dev.i2c_dev.addr = BMP280_I2C_ADDRESS_0;
        bmp280_init(&bmp280_dev, &params);

        while (1) {

                vTaskDelay(pdMS_TO_TICKS(10000));
                temperature_receiver = read_bmp(BMP280_TEMPERATURE);
                pressure_receiver = 0.01*read_bmp(BMP280_PRESSURE);

                printf("Temperature on receiver: %f °C\n", temperature_receiver);
                printf("Pressure on receiver: %f mBar\n", pressure_receiver);

		parse_url(temperature_receiver, pressure_receiver, 0);
		softuart_puts(0, api_string);
        }	

}

extern "C" void user_init(void) {
	uart_set_baud(0, 115200);
	softuart_open(0, 9600, RXD, TXD);

	// Set up connection with AT commands
	softuart_puts(0, "AT+CGCONTRDP");	
	sdk_os_delay_us(second);
	softuart_puts(0, AT_CHTTPCREATE);
	sdk_os_delay_us(second);
	softuart_puts(0, "AT+CHTTPCON=0");

	// Set up the access point
	set_ap();

	i2c_init(BUS_I2C, SCL, SDA, I2C_FREQ_100K);
	gpio_enable(SCL, GPIO_OUTPUT);
	
	xTaskCreate(bmp_task, "Measure data", 512, NULL, 2, NULL);
	xTaskCreate(receive_data_task, "Listen to radio for incoming data", 512, NULL, 2, NULL);
}

