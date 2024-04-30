#include "Dmx.hpp"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_microsleep.h"

static const char*	task_ids[2] = {"SendDmxTask1", "SendDmxTask2"};
static const char*	TAG = "dmx.c";

Dmx	Dmx::universes[DMX_OUTPUT_NBR];

Dmx::Dmx(void) {}

void	Dmx::initialize(void)
{
	esp_microsleep_calibrate();
	for (uint8_t i = 0; i < DMX_OUTPUT_NBR; i++)
		memset(Dmx::universes[i]._channels, 0, sizeof(_channels));

	uart_config_t	config = {
		.baud_rate = 250000,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_2,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 122, //arbitrary
		.source_clk = UART_SCLK_DEFAULT
	};

	Dmx::universes[0]._uart_num = (uart_port_t) DMX_UART_UNIVERSE_1;
	Dmx::universes[0]._tx_pin = DMX_TX_PIN_OUTPUT1;

#if (DMX_OUTPUT_NBR == 2)
	Dmx::universes[1]._uart_num = (uart_port_t) DMX_UART_UNIVERSE_2;
	Dmx::universes[1]._tx_pin = DMX_TX_PIN_OUTPUT2;
#endif

	for (uint8_t i = 0; i < DMX_OUTPUT_NBR; i++)
	{
		uart_param_config(Dmx::universes[i]._uart_num, &config);
		uart_set_pin(Dmx::universes[i]._uart_num, Dmx::universes[i]._tx_pin,
					UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
		uart_driver_install(Dmx::universes[i]._uart_num,
					UART_FIFO_LEN + 10U, 0, 20, NULL, 0);

		Dmx::universes[i]._dataLock = xSemaphoreCreateMutex();
		xTaskCreatePinnedToCore(
			sendTrame, //routine
			task_ids[i], //name
			5096, //stack size
			&Dmx::universes[i], //param
			1, //priority
			NULL, //task handle
			1); //core id
	}
}

void	Dmx::sendTrame(void *param)
{
	uint8_t	start_code = 0;
	Dmx*	dmx = (Dmx*)param;
	while (1)
	{
		ESP_ERROR_CHECK_WITHOUT_ABORT(uart_wait_tx_done(dmx->_uart_num, 1000)); //wait until uart is ready
		// Invert signal (set it to LOW) and wait 110us for break signal (minimum is 90us)
		ESP_ERROR_CHECK_WITHOUT_ABORT(uart_set_line_inverse(dmx->_uart_num, UART_SIGNAL_TXD_INV)); //invert signal from HIGH to LOW
		esp_microsleep_delay(DMX_BREAK_DURATION);
		// Set TX to HIGH for MAB for 12us (minimum 8, recommanded 12us)
		ESP_ERROR_CHECK_WITHOUT_ABORT(uart_set_line_inverse(dmx->_uart_num, 0)); //disable signal ibnvers
		esp_microsleep_delay(DMX_MAB_DURATION);
		// send start code, which is a 0 byte 
		uart_write_bytes(dmx->_uart_num, (const char *)&start_code, 1);
		// DMX protocol allows to wait for undefined time between each frame 
		xSemaphoreTake(dmx->_dataLock, portMAX_DELAY);
		ESP_LOGD(TAG, "wrote %d bytes to UART %d", uart_write_bytes(dmx->_uart_num, dmx->_channels, 512), dmx->_uart_num);
		xSemaphoreGive(dmx->_dataLock);
	}
}

void	Dmx::setChannel(uint16_t index, uint8_t value)
{
	if (index > 512)
		return ;
	xSemaphoreTake(this->_dataLock, portMAX_DELAY);
	this->_channels[index] = value;
	xSemaphoreGive(this->_dataLock);
}

uint8_t	Dmx::getChannel(uint16_t index)
{
	if (index > 512)
		return (0);
	return (this->_channels[index]);
}

/// @brief Copy buffer to DMX buffer. Thread safe
/// @param buffer 
/// @param size 
void	Dmx::setBuffer(uint8_t* buffer, uint16_t size)
{
	xSemaphoreTake(this->_dataLock, portMAX_DELAY);
	memcpy(this->_channels, buffer, size);
	xSemaphoreGive(this->_dataLock);
}
