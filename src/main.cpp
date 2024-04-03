#include "Dmx.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void	setup(void)
{
	// esp_rom_gpio_pad_select_gpio(GPIO_NUM_2);
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	// gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
	// gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
}

void	routine(void *arg)
{
	uint8_t		level = 0;
	uint16_t	*frq = (uint16_t *) arg;
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
	TickType_t	prevTick = xTaskGetTickCount();
	while (1)
	{
		// vTaskDelay(50 / portTICK_PERIOD_MS);
		xTaskDelayUntil(&prevTick, 50 / portTICK_PERIOD_MS);
		if (gpio_get_level(GPIO_NUM_0) == 0 && isPressed == false)
		{
			*frq -= 50;
			isPressed = true;
		}
		else if (isPressed == true)
			isPressed = false;
	}
}

extern "C" void app_main() {
	setup();
	uint16_t frq = 500;
	xTaskCreatePinnedToCore(readButton,
						"ReadingButton",
						2048,
						&frq,
						8,
						NULL,
						0);

	xTaskCreatePinnedToCore(routine,
						"BlinkingLed2",
						2048,
						&frq,
						6,
						NULL,
						0);

	// vTaskDelay(1000000 / portTICK_PERIOD_MS);
	// vTaskStartScheduler();
	while (1)
		continue ;
	return;
}

