#include "Dmx.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Wifi.hpp"
#include "UDPSocket.hpp"
#include "Artnet.hpp"
#include "esp_console.h"

static const char*	TAG = "main.c";
static uint16_t		frq = 500;


void	routine(void *arg)
{
	uint8_t		level = 0;
	uint16_t*	frq = (uint16_t *) arg;
	while (1)
	{
		level %= 2;
		vTaskDelay(*frq / portTICK_PERIOD_MS);
		gpio_set_level(GPIO_NUM_2, level++);
	}
}

void	readButton(void *arg)
{
	bool	isPressed = false;
	uint16_t	*frq = (uint16_t *) arg;
	// TickType_t	prevTick = xTaskGetTickCount();
	while (1)
	{
		vTaskDelay(50 / portTICK_PERIOD_MS);
		// xTaskDelayUntil(&prevTick, 50 / portTICK_PERIOD_MS);
		if (gpio_get_level(GPIO_NUM_0) == 0 && isPressed == false)
		{
			*frq -= 50;
			isPressed = true;
		}
		else if (gpio_get_level(GPIO_NUM_0) == 0)
			continue;
		else
			isPressed = false;
	}
}

void	setup(void)
{
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
}

// void	readLine(void *param)
// {
// 	// uart_intr_config_t	config = {.intr_enable_mask = }
// 	// uart_intr_config(UART_NUM_0, );
// 	char	line[128] = {0};
// 	char	c = 0;

 
// 	while (uart_read_bytes(UART_NUM_0, &c, 1, portMAX_DELAY))
// 	{
// 		uart_write_bytes(UART_NUM_0, &c, 1);
// 	}
// }

static void echo_task(void *arg)
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

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    uart_driver_install(UART_NUM_0, 128 * 2, 0, 0, NULL, intr_alloc_flags);
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(128);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(UART_NUM_0, data, 128, 20 / portTICK_PERIOD_MS);
        // Write data back to the UART
        uart_write_bytes(UART_NUM_0, (const char *) data, len);
    }
}

extern "C" void app_main() {
	(void) TAG;
	
	Wifi::start();

	setup();

	xTaskCreatePinnedToCore(readButton,
						"ReadingButton",
						configMINIMAL_STACK_SIZE + 2048,
						&frq,
						2,
						NULL,
						1);

	xTaskCreatePinnedToCore(routine,
						"BlinkingLed2",
						configMINIMAL_STACK_SIZE + 10,
						&frq,
						1,
						NULL,
						1);

	UDPSocket::start();

	Dmx::initialize();
	Artnet::universeMap[0] = 0;
	UDPSocket::setHandler(Artnet::artnetToDmx); ///Handler should be set after universeMap has been configured
	
	 xTaskCreate(echo_task, "uart_echo_task", configMINIMAL_STACK_SIZE + 10 + 128, NULL, 10, NULL);

	// xTaskCreatePinnedToCore(readLine,
	// 					"Readline",
	// 					configMINIMAL_STACK_SIZE + 10 + 128,
	// 					NULL,
	// 					1,
	// 					NULL,
	// 					3);
}

