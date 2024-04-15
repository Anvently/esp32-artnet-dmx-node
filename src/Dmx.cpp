#include "Dmx.hpp"
#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_microsleep.h"

uint8_t				Dmx::_channels[512] = {0};
SemaphoreHandle_t	Dmx::_dataLock;

static const char*	TAG = "dmx.c";

Dmx::Dmx(void) {}

void	Dmx::initialize(void)
{
	esp_microsleep_calibrate();
	memset(_channels, 0, sizeof(_channels));

	uart_config_t	config = {
		.baud_rate = 250000,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_2,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.rx_flow_ctrl_thresh = 122, //arbitrary
		.source_clk = UART_SCLK_DEFAULT
	};

	uart_param_config(USED_UART, &config);
	uart_set_pin(USED_UART, TX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	QueueHandle_t	uart_queue;
	uart_driver_install(USED_UART, 512, 0, 20, &uart_queue, 0);

	_dataLock = xSemaphoreCreateMutex();
	xTaskCreatePinnedToCore(
		sendTrame, //routine
		"SendDMXTrame", //name
		5096, //stack size
		NULL, //param
		1, //priority
		NULL, //task handle ?
		1); //core id

}

void	Dmx::sendTrame(void *)
{
	uint8_t	start_code = 0;
	while (1)
	{
		ESP_ERROR_CHECK_WITHOUT_ABORT(uart_wait_tx_done(USED_UART, 1000)); //wait until uart is ready
		ESP_ERROR_CHECK_WITHOUT_ABORT(uart_set_line_inverse(USED_UART, UART_SIGNAL_TXD_INV)); //invert signal
		esp_microsleep_delay(110);
		ESP_ERROR_CHECK_WITHOUT_ABORT(uart_set_line_inverse(USED_UART, 0)); //invert for mark after break
		esp_microsleep_delay(12);
		uart_write_bytes(USED_UART, (const char *)&start_code, 1); // send start code
		xSemaphoreTake(_dataLock, portMAX_DELAY);
		// ESP_LOGI(TAG, "wrote %d bytes", uart_write_bytes(USED_UART, _channels, 512));
		xSemaphoreGive(_dataLock);
	}
}

void	Dmx::setChannel(uint16_t index, uint8_t value)
{
	if (index > 512)
		return ;
	xSemaphoreTake(_dataLock, portMAX_DELAY);
	_channels[index] = value;
	xSemaphoreGive(_dataLock);
}

uint8_t	Dmx::getChannel(uint16_t index)
{
	if (index > 512)
		return (0);
	return (_channels[index]);
}
