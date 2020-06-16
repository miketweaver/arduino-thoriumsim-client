#include "communication.h"

HttpRequest httpReq;

EthernetServer server(80); //server port arduino server will use
EthernetClient client;
HttpClient httpclient = HttpClient(client, '127.0.0.1', 80); // temp data until ethernet_setup

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
    Serial.println(Ethernet.localIP());
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

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println(F("<!DOCTYPE html><HTML><HEAD>"));
          client.println(F("<TITLE>Fire Suppression System Panel</TITLE>"));
          client.println(F("</HEAD>"));
          client.println(F("<BODY>"));
          
          client.println(F("<h2>Fire Suppression System Panel</h2>"));
          client.println(F("<form action=\"/\" method=\"get\">"));
          client.println(F("<label for=\"simulator\">Choose a simulator:</label>"));
          client.println(F("<select id=\"simulator\" name=\"simulator\">"));
          client.println(F("<option value=\"1\">d2ca23bd-d6a3-443c-a2b1-ee8b4592101b</option>"));
          client.println(F("<option value=\"2\">1111</option>"));
          client.println(F("</select>"));
          client.println(F("<input type=\"submit\">"));
          client.println(F("</form>"));

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
          client.println("<br>");
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
          client.println("Cookies:<br>");
          for(int i=1;i<=httpReq.cookieCount;i++){
            httpReq.getCookie(i,name,value);
            client.print(name);
            client.print(" - ");
            client.print(value);
            client.println("<br>");
          }
          //find a particular parameter name
          int pos=httpReq.getParam("MyParameter",value);
          if(pos>0){
            client.print("<br>");
            client.print("Found 'MyParameter'. Value: ");
            client.print(value);
            client.println("<br>");  
          }
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
