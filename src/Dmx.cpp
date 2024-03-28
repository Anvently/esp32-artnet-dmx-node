#include "Dmx.hpp"

Dmx::Dmx(void) {}

void	Dmx::initialize(void)
{
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
	uart_driver_install(USED_UART, 2048, 2048, 20, &uart_queue, 0);

	_dataLock = xSemaphoreCreateMutex();
	xTaskCreatePinnedToCore(
		sendTrame, //routine
		"SendDMXTrame", //name
		2048, //stack size
		NULL, //param
		1, //priority
		NULL, //task handle ?
		1); //core id

}

void	Dmx::sendTrame(void *)
{
	while (1)
	{
		uart_wait_tx_done(USED_UART, 1000); //wait until uart is ready
		uart_set_line_inverse(USED_UART, UART_SIGNAL_TXD_INV); //invert signal
		vTaskDelay(110000 / portTICK_PERIOD_MS); //wait during break
		uart_set_line_inverse(USED_UART, 0); //invert for mark after break
		vTaskDelay(12000 / portTICK_PERIOD_MS); //wait for MAB
		uart_write_bytes(USED_UART, (const char *)0, 1); // send start code
		xSemaphoreTake(_dataLock, portMAX_DELAY);
		uart_write_bytes(USED_UART, _channels, 512);
		xSemaphoreGive(_dataLock);
	}
}

void	Dmx::setChannel(uint16_t index, uint8_t value)
{
	if (index > 512)
		return ;
	_channels[index] = value;
}

uint8_t	Dmx::getChannel(uint16_t index)
{
	if (index > 512)
		return (0);
	return (_channels[index]);
}
