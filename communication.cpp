#include "communication.h"

HttpRequest httpReq;

EthernetServer server(80); //server port arduino server will use
EthernetClient client;
HttpClient httpclient = HttpClient(client, '127.0.0.1', 80); // temp data until ethernet_setup

#ifndef ESP32
void(* resetFunc) (void) = 0;//declare reset function at address 0
#endif

void ethernet_setup(char* thorium_server, int thorium_port, IPAddress ip, IPAddress myDns, byte mac[], int cs_pin){
  httpclient = HttpClient(client, thorium_server, thorium_port);
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(cs_pin);

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.print(Ethernet.localIP());
    Serial.print(" DNS: ");
    Serial.println(Ethernet.dnsServerIP());

  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void ethernet_loop() {
  ethernet_client_loop();
  ethernet_server_loop();
}

void ethernet_server_loop() {
  //declare name and value to use the request parameters and cookies
  char name[HTTP_REQ_PARAM_NAME_LENGTH], value[HTTP_REQ_PARAM_VALUE_LENGTH];
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        //parse the received caracter
        httpReq.parseRequest(c);
        
        Serial.write(c);
        
        //IF request has ended -> handle response
        if (httpReq.endOfRequest()) {

          if(String(httpReq.uri).startsWith("/reset")) { 
#ifdef ESP32
            ESP.restart();
#else
            resetFunc();
#endif
          }

          //act on parameters
          int pos=httpReq.getParam("panelName",value);
          if(pos>0){
            String name = value;
            name.replace("+", " ");
            setPanelName(name);
          }
          pos=httpReq.getParam("serverName",value);
          if(pos>0){
            String name = value;
            setThoriumServer(name);
          }
          pos=httpReq.getParam("serverPort",value);
          if(pos>0){
            unsigned int port = atoi(value);
            setThoriumPort(port);
          }
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println(F("<!DOCTYPE html><HTML><HEAD>"));
          client.print(F("<TITLE>"));
          client.print(thoriumInfo.panelName);
          client.println(F("</TITLE>"));
          client.println(F("<style>.collapsible {background-color: #777;color: white;cursor: pointer;padding: 18px;}.content {padding: 0 18px;display: none;overflow: hidden;background-color: #f1f1f1;}</style>"));
          client.println(F("</HEAD>"));
          client.println(F("<BODY>"));
          client.print(F("<h2>"));
          client.print(thoriumInfo.panelName);
          client.println(F("</h2>"));
          client.println(F("<form action=\"/\" method=\"get\">"));
          client.println(F("<label for=\"panelName\">Panel Name:</label>"));
          client.print(F("<input type=\"text\" id=\"panelName\" name=\"panelName\" value=\""));
          client.print(thoriumInfo.panelName);
          client.println(F("\"><br><label for=\"serverName\">Thorium Server Hostname/IP:</label>"));
          client.print(F("<input type=\"text\" id=\"serverName\" name=\"serverName\" value=\""));
          client.print(thoriumInfo.serverName);
          client.println(F("\"><br><label for=\"serverPort\">Thorium Server Port (default 3001):</label>"));
          client.print(F("<input type=\"number\" id=\"serverPort\" name=\"serverPort\" value=\""));
          client.print(thoriumInfo.serverPort);
          client.println(F("\"><br>Current Simulator: "));
          client.print(thoriumInfo.simulatorId);
          client.println(F("<br>"));
          client.print(F("Current Flight: "));
          client.print(thoriumInfo.flightId);
          client.println(F("<br>"));
          client.print(F("Current ClientId: "));
          client.print(thoriumInfo.clientId.string);
          // client.println(F("<label for=\"simulator\">Choose a simulator:</label>"));
          // client.println(F("<select id=\"simulator\" name=\"simulator\">"));
          // client.println(F("<option value=\"1\">d2ca23bd-d6a3-443c-a2b1-ee8b4592101b</option>"));
          // client.println(F("<option value=\"2\">1111</option>"));
          // client.println(F("</select>"));
          client.println(F("<br><input type=\"submit\" value=\"Submit\">"));

          client.println(F("</form><p>You must reset your panel for Thorium Server Name or Port changes</p>"));
          client.println(F("<form action=\"/reset\" method=\"get\"><input type=\"submit\" value=\"Reset Panel\" /></form>"));
          client.println(F("<br><button type=\"button\" class=\"collapsible\">Click for Debugging Info</button><div class=\"content\">"));


          //access object properties
          client.print("Method: ");
          client.print(httpReq.method);
          client.println("<br>");
          client.print("Uri: ");
          client.print(httpReq.uri);
          client.println("<br>");
          client.print("Version: ");
          client.print(httpReq.version);
          client.println("<br>");
          client.print("paramCount: ");
          client.print(httpReq.paramCount);
          client.println("<br><br>");
          //list received parameters GET and POST
          client.println("Parameters:<br>");
          for(int i=1;i<=httpReq.paramCount;i++){
            httpReq.getParam(i,name,value);
            client.print(name);
            client.print(": ");
            client.print(value);
            client.println("<br>");
          }
          //list received cookies
          client.println("<br>Cookies:<br>");
          for(int i=1;i<=httpReq.cookieCount;i++){
            httpReq.getCookie(i,name,value);
            client.print(name);
            client.print(" - ");
            client.print(value);
            client.println("<br>");
          }
          client.println(F("</div><script>var coll = document.getElementsByClassName(\"collapsible\");var i;for (i = 0; i < coll.length; i++) {coll[i].addEventListener(\"click\", function() {this.classList.toggle(\"active\");var content = this.nextElementSibling;if (content.style.display === \"block\") {content.style.display = \"none\";} else {content.style.display = \"block\";}});}</script>"));
          client.println("</body>");
          client.print("</html>");
          
          //Reset object and free dynamic allocated memory
          httpReq.resetRequest();
          
          break;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void ethernet_client_loop() {

}
