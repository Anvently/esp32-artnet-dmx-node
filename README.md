# ESP Artnet to Dmx node

ESP32 and esp-idf based project allowing a controller to receive Artnet Dmx512 data via Wifi and drive up to 2 potential DMX output using MAX485 modules.

## Wifi configuration

Default SSID and password can be configured in Wifi.hpp, and also at runtime (see USB configuration). When disconnected, Wifi routine should try to reconnect undefinitely to the configured network, and UDP socket should restart on its own when connected to a new network. DMX data will keep transmitting the last frame received.

## DMX output

To drive the DMX output, 2 of the 3 available ESP32's UART interface are used (UART2 and UART3).
UART0 is reserved for logging/debugging and to control and configure the program.
Number of used DMX output (from 1 to 2), used TX pins and default universe mapping can be configured in Dmx.hpp.
Default pin are GPIO2 for output 1 and GPIO4 for output 2. By default, output1 is mapped to universe 0 and output2 is mapped to universe 1.
A single universe can be mapped to both outputs. In this case the same data will be transmitted to both DMX devices.
Each output should be connected to a MAX485 module connected to a DMX/XLR output (see schematic).

## USB configuration via UART0

Wifi configuration and universe mapping can be configured at runtime via a very rudimentary console using a serial console (ex: platformio's serial monitor or "screen" program on ubuntu);

There is currently 3 available command :
- ip
   => print the local ip address of the artnet node
- wifi [ssid] [password]
   => connect to a new wifi network
- universe [universe nbr] [output number]
   => map a given universe to a given output. Every packet intended for this universe will be redirected to the set output.

ESC key can be used to toggle LOGs and makes it easier to input commands.
Disabling the log seems to trigger FreeRTOS watchdog for some reason, but it should not impact program behaviour.
