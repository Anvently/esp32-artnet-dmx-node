#ifndef UDPSOCKET_HPP
# define UDPSOCKET_HPP

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#define UDP_PORT 6454
#define BLOCK_TIMEOUT_SEC 1000
#define SOCKET_BUFFER_SIZE 1024

class UDPSocket
{
	private:

		UDPSocket(void);
		~UDPSocket(void);

		static void			printBuffer(uint32_t len);

		static unsigned char		rx_buffer[SOCKET_BUFFER_SIZE];
		static uint32_t				hostIP; //A mutex could be needed for this variable in case of disconnect/reconnect
		static uint8_t				(*handler)(const char*, uint16_t);


	public:

		static void			start(void);
		static void			socketTask(void *arg);
		static void			setHandler(uint8_t (*handler)(const char*, uint16_t));
		
};


#endif