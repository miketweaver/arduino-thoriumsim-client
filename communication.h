#ifndef _COMMUNICATION_H 
#define _COMMUNICATION_H

#include <SPI.h>
#include <Ethernet.h>
//include HttpRequest object library https://forum.arduino.cc/index.php?topic=197752.msg1506850#msg1506850
#include <HttpRequest.h>
#include <ArduinoHttpClient.h>
#include "Thorium.h"

extern HttpRequest httpReq;
extern EthernetServer server;
extern EthernetClient client;
extern HttpClient httpclient;

void ethernet_setup(char* thorium_server, int thorium_port, IPAddress ip, IPAddress myDns, byte mac[], int cs_pin);
void ethernet_loop();
void ethernet_server_loop();
void ethernet_client_loop();

#endif // _COMMUNICATION_H