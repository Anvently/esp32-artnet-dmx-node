#include "Dmx.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Wifi.hpp"
#include "UDPSocket.hpp"
#include "Artnet.hpp"
#include "esp_console.h"
#include "utils.hpp"

static const char*	TAG = "main.c";

static void	universe_command(char* line)
{
	uint16_t	output, universe;
	skip_space(&line);
	if (parse_int(&line, &universe, ' '))
		ESP_LOGE(TAG, "Invalid format for universe, try: universe [nbr] [1::2(dmx_output)]");
	skip_space(&line);
	if (parse_int(&line, &output, '\0'))
		ESP_LOGE(TAG, "Invalid format for output, try: universe [nbr] [1::2(dmx_output)]");
	if (output > DMX_OUTPUT_NBR || output < 1)
		ESP_LOGE(TAG, "Output is invalid, it must be between 1 and %d. "\
				"Update output nbr in Dmx.hpp to have more than 1 output", DMX_OUTPUT_NBR);
	Artnet::universeMap[output] = universe;
}


static void	wifi_command(char* line)
{
	uint8_t		ssid[32] = {0}, password[64] = {0};
	char		*ssid_start, *password_start;
	uint16_t	ssid_size, password_size;
	ssid_size = parse_word(line, &line, &ssid_start);
	if (ssid_size < 1)
	{
		ESP_LOGE(TAG, "Invalid format. Try : wifi [ssid] [password]");
		return;
	}
	password_size = parse_word(line, &line, &password_start);
	if (password_size < 1)
	{
		ESP_LOGE(TAG, "Invalid format. Try : wifi [ssid] [password]");
		return;
	}
	if (ssid_size > 31 || password_size > 63)
		ESP_LOGE(TAG, "SSID must be a maximum of 31 characters and password 63 characters.");
	else
	{
		memcpy(ssid, ssid_start, ssid_size);
		ssid[ssid_size] = '\0';
		memcpy(password, password_start, password_size);
		password[password_size] = '\0';
		Wifi::setNetwork(ssid, password);
	}
}

static void parse_line(char* line)
{
	skip_space(&line);
	if (strncmp(line, "wifi ", 5) == 0) //If wifi command
		wifi_command(line + 5);
	else if (strncmp(line, "universe ", 9) == 0) //If universe command
		universe_command(line + 9);
	else if (strncmp(line, "ip", 2) == 0)
		Wifi::printHostIP();
	else
	{
		ESP_LOGE(TAG, "Invalid command. Possible command are : \n"\
						"universe [nbr] [1::2(dmx_output)]\n"\
						"wifi [ssid] [password]\n"\
						"ip");
	}
}

static void	get_next_line(char *buffer, uint16_t size, const char* prompt)
{
	char			c = 0;
	uint16_t		index = 0;
	int8_t			len = 0;
	bool			log = true;

	if (prompt)
		uart_write_bytes(UART_NUM_0, prompt, sizeof(prompt));
	do
	{
		// ESP_LOGI(TAG, "%hhu", c);
		len = uart_read_bytes(UART_NUM_0, &c, 1, 20 / portTICK_PERIOD_MS);
		if (len == 0)
			continue;
		else if (len < 0)
			ESP_LOGE(TAG, "Error reading new line");
		else if (c == 127 && index > 0) //If char is DELETE
		{
			buffer[index--] = '\0';
			uart_write_bytes(UART_NUM_0, "\b \b", 4);
		}
		else if (c == 27) //If char is ESC => toggle log
		{
			log = !log;
			if (log == true)
			{
				ESP_LOGE(TAG, "LOG ENABLED");
				esp_log_level_set("*", ESP_LOG_INFO);
			}
			else
			{
				ESP_LOGE(TAG, "LOG DISABLED");
				esp_log_level_set("*", ESP_LOG_ERROR);
			}
		}
		else if (c == '\r')
			uart_write_bytes(UART_NUM_0, "\n\r", 3);
		else if (c < 32 || c > 126 || index + 1 == size) {}
		else if (index + 1 < size)
		{
			buffer[index++] = c;
			uart_write_bytes(UART_NUM_0, &c, 1);
		}
	} while (c != '\r');
	buffer[index] = '\0';
}

static void console_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

    uart_driver_install(UART_NUM_0, 128 * 2, 0, 0, NULL, intr_alloc_flags);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Configure a temporary buffer for the incoming data
    char	line[128] = {0};

    while (1) {
		get_next_line(line, 128, "cmd:");
		parse_line(line);
    }
}

extern "C" void app_main() {
	(void) TAG;
	
	Wifi::start();
	UDPSocket::start();

	Dmx::initialize();
	Artnet::universeMap[0] = 0;
	UDPSocket::setHandler(Artnet::artnetToDmx); ///Handler should be set after universeMap has been configured
	
	 xTaskCreate(console_task, "consoleTask", configMINIMAL_STACK_SIZE + 10 + 1048, NULL, 10, NULL);
}

