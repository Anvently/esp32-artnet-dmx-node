#ifndef SERIALIZER_H
#define SERIALIZER_H

#pragma once

#include "freertos/FreeRTOS.h"

class Serializer
{
	public:

		static void	sendByte(uint8_t outPin, uint8_t data, uint8_t baud);

	private:

		Serializer();
		~Serializer();

};

#endif