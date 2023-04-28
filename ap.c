/*
	Make an access point to which we can connect.
	It dynamically allocates an IP address to anyone connecting.
*/

#include <string.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <dhcpserver.h>
#include "nrf24.h"

// CREDENTIALS
#define SSID "acc-point"
#define PASS "passwd"

#define GPIO 2

// NRF
#define CE_NRF 3
#define CS_NRF 0
#define channel 33

/*
void dhcps_lease_test(void)
{
	struct dhcps_lease {
		struct ip_addr start_ip;
		struct ip_addr end_ip;
	};
	struct dhcps_lease lease;
	IP4_ADDR(&lease.start_ip, 192, 168, 0, 10);
	IP4_ADDR(&lease.end_ip, 192, 168, 0, 20);
	sdk_wifi_softap_set_dhcps_lease(&lease);	
}
*/

void set_ap(void) {
	
	sdk_wifi_set_opmode(SOFTAP_MODE);

	struct ip_info info;
	IP4_ADDR(&info.ip, 192, 168, 0, 1);
	IP4_ADDR(&info.gw, 192, 168, 0, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	sdk_wifi_set_ip_info(1, &info);

	struct sdk_softap_config config = {
		.ssid = SSID,
		.ssid_len = strlen(SSID),
		.password = PASS,
		.authmode = AUTH_WPA_WPA2_PSK
	}; 
	sdk_wifi_softap_set_config(&config);
	
	ip_addr_t first_client_ip;
	IP4_ADDR(&first_client_ip, 192, 168, 0, 2);
	dhcpserver_start(&first_client_ip, 4);
	dhcpserver_set_dns(&info.ip);
	dhcpserver_set_router(&info.ip);

	printf("DHCP started\n");
}

void blink_led_on_traffic(void *pvParameters) {
	while (1) {
		if (radio.available()) {
			gpio_write(GPIO, 1);
			vTaskDelay(pdMS_TO_TICKS(200));
			gpio_write(GPIO, 0);
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void user_init(void) {
	uart_set_baud(0, 115200);
	
	printf("SDK version: %s\n", sdk_system_get_sdk_version());

	set_ap();
	
	radio.openReadingPipe(1, address);
	radio.startListening();
	xTaskCreate(&blink_led_on_traffic, "Blink LED", 512, NULL, 3, NULL);	
}

