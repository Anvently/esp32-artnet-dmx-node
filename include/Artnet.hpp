#ifndef ARTNET_HPP
# define ARTNET_HPP

# include "driver/uart.h"
# include "freertos/FreeRTOS.h"
# include "freertos/task.h"
# include "freertos/queue.h"
# include "freertos/semphr.h"
# include "freertos/event_groups.h"
# include "string.h"
# include "Dmx.hpp"

# define ARTNET_ERROR_INVALID_PACKET 1
# define ARTNET_


class Artnet
{

	private:

		~Artnet(void);
		Artnet(void);

	public:

		static uint16_t	universeMap[DMX_OUTPUT_NBR];

		/// @brief Check if the buffer is Artnet and if so parse it and send it to DMX buffer
		/// @param buffer 
		/// @param size 
		static uint8_t	artnetToDmx(const char* buffer, uint16_t size);

};

#endif