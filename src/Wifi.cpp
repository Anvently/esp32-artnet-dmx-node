#include "Wifi.hpp"

static const char* TAG = "Wifi";

uint8_t				Wifi::reconnectTries = 0;
EventGroupHandle_t	Wifi::wifiEvents;
uint32_t			Wifi::hostIP = 0;
SemaphoreHandle_t	Wifi::connectionLock;
bool				Wifi::isConnected = false;

void	Wifi::eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *data)
{
	//If wifi ready
		//try to connect
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
		esp_wifi_connect();
	//If wifi disconnected
		//try to reconnect for max try
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		xEventGroupClearBits(Wifi::wifiEvents, WIFI_CONNECTED_BIT);
		xEventGroupSetBits(Wifi::wifiEvents, WIFI_DISCONNECT_BIT);
		if (reconnectTries >= WIFI_MAX_RECONNECT)
		{
			xEventGroupSetBits(Wifi::wifiEvents, WIFI_FAIL_BIT);
			return ;
		}
		reconnectTries++;
		ESP_LOGI(TAG, "disconnected. trying to reconnect");
		esp_wifi_connect();
	}
	//If wifi connected
		//start application
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		Wifi::hostIP = ((ip_event_got_ip_t *)data)->ip_info.ip.addr;
		ESP_LOGI(TAG, "wifi connected, ip = " IPSTR, \
			IP2STR(&((ip_event_got_ip_t *)data)->ip_info.ip));
		xEventGroupClearBits(Wifi::wifiEvents, WIFI_DISCONNECT_BIT);
		xEventGroupClearBits(Wifi::wifiEvents, WIFI_FAIL_BIT);
		xEventGroupSetBits(Wifi::wifiEvents, WIFI_CONNECTED_BIT);
		reconnectTries = 0;
	}
}

esp_err_t	Wifi::init(void)
{
	esp_err_t	ret = ESP_OK;
	wifi_init_config_t	config = WIFI_INIT_CONFIG_DEFAULT();
	esp_event_handler_instance_t	instance_got_ip;
	wifi_config_t		wifi_config = {
		.sta = {
			.ssid = WIFI_SSID,
			.password = WIFI_PASSWORD,
			.threshold = {
					.authmode = WIFI_AUTH_WEP  //Minimum security allowed is WEP,
			}
		}
	};

	// Initialize NVS
	ESP_ERROR_CHECK(nvs_flash_init());
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ESP_ERROR_CHECK(nvs_flash_init());
	}

	Wifi::connectionLock = xSemaphoreCreateMutex();

	Wifi::wifiEvents = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_netif_init()); //starting netif task
	ESP_ERROR_CHECK(esp_event_loop_create_default()); //start the event loop for handling wifi events
	esp_netif_create_default_wifi_sta(); 
	ESP_ERROR_CHECK(esp_wifi_init(&config)); //start wifi driver task
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler, NULL, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &eventHandler, NULL, &instance_got_ip));

	return (ret);
}

/// @brief Block task until a wifi connexion is established
/// @param  
void	Wifi::waitConnexion(void)
{
	xEventGroupWaitBits(Wifi::wifiEvents, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

void	Wifi::wifiConnexionTask(void *)
{
	EventBits_t			bit;

	ESP_ERROR_CHECK(esp_wifi_start());
	while (1)
	{
		bit = xEventGroupWaitBits(Wifi::wifiEvents, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
		if (bit & WIFI_FAIL_BIT)
		{
			xEventGroupClearBits(Wifi::wifiEvents, WIFI_FAIL_BIT);
			ESP_LOGI(TAG, "Connection failed to %s with password: %s", WIFI_SSID, WIFI_PASSWORD);
			// xEventGroupClearBits(Wifi::wifiEvents, WIFI_FAIL_BIT);
			esp_wifi_connect();
		}
		else if (bit & WIFI_CONNECTED_BIT || bit & WIFI_DISCONNECT_BIT)
		{
			vTaskDelay(pdMS_TO_TICKS(10));
		}
		else
		{
			ESP_LOGE(TAG, "Unexpected event");
			abort();
		}
	}
}

void	Wifi::start(void)
{
	ESP_LOGI(TAG, "initializing wifi");
	Wifi::init();
	ESP_LOGI(TAG, "starting wifi");
	xTaskCreatePinnedToCore(&Wifi::wifiConnexionTask,
						"ConnexionTask",
						configMINIMAL_STACK_SIZE + 5096,
						NULL,
						2,
						NULL,
						1);
	vTaskDelay(pdMS_TO_TICKS(10)); //Small wait to allow connexion task to retrieve args
}

/// @brief Return current IP of wifi interface
/// @param  
/// @return 
uint32_t	Wifi::getHostIP(void)
{
	return (Wifi::hostIP);
}
