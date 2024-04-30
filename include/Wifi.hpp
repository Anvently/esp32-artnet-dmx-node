#ifndef WIFI_HPP
# define WIFI_HPP

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"


#define WIFI_MAX_RECONNECT 3
#define WIFI_SSID "default_ssid"
#define WIFI_PASSWORD "default_password"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_DISCONNECT_BIT BIT2

/*

1. Wifi scan
	- WIFI_EVENT_SCAN_DONE
		is generated after a call to esp_wifi_scan_start()
		can occur after a call to restart the scan or the explicitely stop the scan
		nothing is done : esp_wifi_scan_get_ap_num/record() must be call to get rslt of the scan
			and allow wifi driver to free allocated memory
	- WIFI_EVENT_STA_START
		is arised if esp_wifi_sta_start() returns esp_ok
		netif interface is started
	- WIFI_EVENT_STA_CONNECTED
		is arise if esp_wifi_sta_connect() returns esp_ok
		DHCP client is started
		main application can then do is job 
	- WIFI_EVENT_STA_DISCONNECTED
		can be arised by user (esp_wifi_disconnect(), esp_wifi_stop())
		can be arised if esp_wifi_connect() fails
		if connection is lost for some reason
		the netif interface is shutdown and udp/tcp connections are closed
Acces point mode:
	- WIFI_EVENT_AP_START
	- WIFI_EVENT_AP_STOP
	- WIFI_EVENT_AP_STACONNECTE

*/
class Wifi
{
	private:

		Wifi(void);
		~Wifi(void);

		static void	eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data);

		static esp_err_t	init(void);
		static void			wifiConnexionTask(void *arg);

		static uint8_t				reconnectTries;
		static EventGroupHandle_t	wifiEvents;
		static esp_ip4_addr_t		hostIP; //A mutex could be needed for this variable in case of disconnect/reconnect
		static SemaphoreHandle_t	connectionLock;
		static bool					isConnected;
		static TaskHandle_t			taskHandle;
		static wifi_config_t		wifiConfig;

	public:

		static void			start(void);
		static void			waitConnexion(void);
		static uint32_t		getHostIP(void);
		static void			printHostIP(void);
		static void			stop(void);
		static void			setNetwork(uint8_t ssid[32], uint8_t password[64]);
		
};


#endif