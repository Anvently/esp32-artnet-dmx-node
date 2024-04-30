#ifndef DMX_HPP
#define DMX_HPP

#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "string.h"

#define DMX_OUTPUT_NBR 2
#define DMX_TX_PIN_UNIVERSE_1 2
#define DMX_TX_PIN_UNIVERSE_2 4
#define DMX_UART_UNIVERSE_1 UART_NUM_1
#define DMX_UART_UNIVERSE_2 UART_NUM_2
#define FRQ_DFT 1000000
#define DMX_BREAK_DURATION 110
#define DMX_MAB_DURATION 12

class Dmx
{

	private:

		Dmx(void);

		uint8_t						_channels[512];
		SemaphoreHandle_t			_dataLock;
		static TaskHandle_t			_taskHandle;
		uart_port_t					_uart_num;
		int							_tx_pin;

		static void					sendTrame(void *params);


	public:
		
		static Dmx					universes[DMX_OUTPUT_NBR];

		static void					initialize(void);

		void						setChannel(uint16_t index, uint8_t value);
		uint8_t						getChannel(uint16_t index);

		void						setBuffer(uint8_t*	buffer, uint16_t size);

};

#endif