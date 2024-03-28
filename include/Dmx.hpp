#ifndef DMX_HPP
#define DMX_HPP

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"

#define TX_PIN 2
#define USED_UART 2
#define FRQ_DFT 1000000


class Dmx
{

	private:

		Dmx(void);

		static uint8_t				_channels[512];
		static SemaphoreHandle_t	_dataLock;

		static void					sendTrame(void *params);


	public:
		
		static void	initialize(void);

		static void		setChannel(uint16_t index, uint8_t value);
		static uint8_t	getChannel(uint16_t index);

};

#endif