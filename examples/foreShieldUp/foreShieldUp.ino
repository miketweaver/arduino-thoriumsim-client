#include <Thorium.h>

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
// IPAddress ip(192, 168, 0, 10);
char thorium_server[] = "192.168.0.10";    // thorium server address
int thorium_port = 3001;
int w5500_cs_pin = 13;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 5);
IPAddress myDns(192, 168, 0, 1);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Setup Devices
  ethernet_setup(thorium_server, thorium_port, ip, myDns, mac, w5500_cs_pin);
  thorium_setup();
}

void loop() {
  // check for serial input
  if (Serial.available() > 0)
  {
    byte inChar;
    inChar = Serial.read();
    if(inChar == 's') // wait for user to send letter `s` over serial cable
    {
        // get shield ID
        // 1 = fore
        int position = 1;
        String query = "query a($simulator: ID) {shields(simulatorId: $simulator) {id, position}}";
        String json = graphql_request(query);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);
        JsonArray array = doc["data"]["shields"].as<JsonArray>();
        String shieldId;
        for(JsonVariant shield : array) {
            if(shield["position"].as<int>() == position) {
              shieldId = shield["id"].as<String>();
            }
        }

        // turn shield on
        query = "mutation shieldUp($shieldId: ID!) {shieldRaised(id: $shieldId)}";
        String variables = "\"shieldId\": \"" + shieldId + "\"";
        graphql_request(query, variables);
    }
  }
  
  ethernet_loop();
  thorium_loop();
}
