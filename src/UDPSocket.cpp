#include "UDPSocket.hpp"
#include "Wifi.hpp"

static const char* TAG = "UDPSocket";

uint32_t			UDPSocket::hostIP = 0;


void	UDPSocket::socketTask(void *)
{
	char	rx_buffer[512];
	// char	host_ip[] = "192.168.0.19";
	int		addr_family = AF_INET;
	int		ip_protocol = IPPROTO_IP;
	
	
	while (1)
	{
		ESP_LOGI(TAG, "Waiting for IP attribution from Wifi");
		Wifi::checkConnexion();
		ESP_LOGI(TAG, "IP found, starting socket, with timeout set as %d", BLOCK_TIMEOUT_SEC);

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
				//check if artnet packet
				//...
				ESP_LOGI(TAG, "received %d bytes", bytesReceived);
			}
			vTaskDelay(pdMS_TO_TICKS(1)); //May not be usefull
		}
	}
}

void	UDPSocket::start(void)
{
	ESP_LOGI(TAG, "initializing udp socket task");
	xTaskCreatePinnedToCore(&UDPSocket::socketTask,
						"SocketTask",
						configMINIMAL_STACK_SIZE + 4096,
						NULL,
						2,
						NULL,
						1);
}
