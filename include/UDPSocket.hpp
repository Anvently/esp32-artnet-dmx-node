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

#define UDP_PORT 12345
#define BLOCK_TIMEOUT_SEC 1000

class UDPSocket
{
	private:

		UDPSocket(void);
		~UDPSocket(void);

		static uint32_t				hostIP; //A mutex could be needed for this variable in case of disconnect/reconnect

	public:

		static void			start(void);
		static void			socketTask(void *arg);
		
};


#endif