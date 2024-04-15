#include "Serializer.h"
#include "driver/gpio.h"

Serializer::Serializer() {}

void	Serializer::sendByte(uint8_t outPin, u_int8_t data, u_int8_t baud)
{
	(void) outPin;
	(void) data;
	(void) baud;
}
