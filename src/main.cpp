#include "Dmx.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Wifi.hpp"
#include "UDPSocket.hpp"

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
	for (int i = 0; i < 512; i++)
		Dmx::setChannel(i, 255);
	ESP_LOGI(TAG, "channel written");

}

