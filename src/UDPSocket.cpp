#include "UDPSocket.hpp"
#include "Wifi.hpp"

static const char* TAG = "UDPSocket";

uint32_t			UDPSocket::hostIP = 0;
unsigned char		UDPSocket::rx_buffer[SOCKET_BUFFER_SIZE] = {0};
uint8_t				(*UDPSocket::handler)(const char*, uint16_t) = NULL;

void	UDPSocket::printBuffer(uint32_t len)
{
	printf("Header: %s \n", rx_buffer);
	printf("opcode: %hu \n", (uint16_t)*(rx_buffer + 8));
	printf("version: %hu \n", (uint16_t)*(rx_buffer + 10));
	printf("sequence: %hhu \n", (uint8_t)*(rx_buffer + 12));
	printf("physical port: %hhu \n", (uint8_t)*(rx_buffer + 13));
	printf("universe: %hu \n", (uint16_t)*(rx_buffer + 14));
	printf("length in bytes: %hu \n", (uint16_t)*(rx_buffer + 16));

	for (uint32_t i = 18; i < len; i++)
	{
		printf("%hhu ", rx_buffer[i]);
	}
	printf("\n");
}

void	UDPSocket::socketTask(void *)
{
	// char	host_ip[] = "192.168.0.19";
	int		addr_family = AF_INET;
	int		ip_protocol = IPPROTO_IP;

	while (1)
	{
		ESP_LOGI(TAG, "Waiting for IP attribution from Wifi");
		Wifi::waitConnexion();
		ESP_LOGI(TAG, "IP set, starting socket, with timeout set as %d", BLOCK_TIMEOUT_SEC);

		struct sockaddr_in client_addr;
		client_addr.sin_addr.s_addr = Wifi::getHostIP();
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(UDP_PORT);

		int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
		if (sock < 0)
		{
			ESP_LOGE(TAG, "Couldn't create socket : errno = %d", errno);
			abort();
		}

		if (bind(sock, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
		{
			ESP_LOGE(TAG, "Couldn't bind socket : errno = %d", errno);
			abort();
		}

		struct timeval	timeout = {
			.tv_sec = BLOCK_TIMEOUT_SEC,
			.tv_usec = 0
		};
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
		ESP_LOGI(TAG, "socket created");
		while (1)
		{
			struct sockaddr_storage	src_addr;
			socklen_t				socklen = sizeof(src_addr);
			int bytesReceived = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (sockaddr *)&src_addr, &socklen);
			
			if (bytesReceived < 0)
			{
				ESP_LOGE(TAG, "the socket failed to receive, restarting socket: errno = %d", errno);
				close(sock);
				sock = -1;
				break;
			}
			else
			{
				ESP_LOGD(TAG, "received %d bytes", bytesReceived);
				if (handler && handler((const char*)rx_buffer, bytesReceived))
					ESP_LOGD(TAG, "Invalid packet was received");
					
			}
			vTaskDelay(pdMS_TO_TICKS(1)); //May not be usefull
		}
	}
}

/// @brief Set the handler function that will be call each time the
/// socket receives a packet. 
/// @param uint16_t 
void	UDPSocket::setHandler(uint8_t (*handler)(const char*, uint16_t))
{
	UDPSocket::handler = handler;
}

void	UDPSocket::start(void)
{
	ESP_LOGI(TAG, "initializing udp socket task");

	UDPSocket::handler = handler;

	xTaskCreatePinnedToCore(&UDPSocket::socketTask,
						"SocketTask",
						configMINIMAL_STACK_SIZE + 4096,
						NULL,
						2,
						NULL,
						1);
}
