#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Wake Light";
const char* password = "stayinbed!";
const int timeout = 5000;  // max response waiting period

String input = "";
String request = "";

ESP8266WebServer server(80);
SoftwareSerial arduino(0,2); // RX, TX

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

String getResponse() 
{
  String str = "";
  int now = millis();
  
  // start timeout counter
  while(!arduino.available()) 
  {
    if((millis() - now) > timeout) 
    {
      str = "{\"msg\":\"Server Timeout :( \"}";
      return str;
    }
  }
  
  while (arduino.available() > 0)
  {
    str = arduino.readString(); // Read serial
  }
  return str;
}


String sendRequest(String request) 
{
  arduino.print(request);
}


void setup(void){

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  
  arduino.begin(57600);
  arduino.setTimeout(250);
  
//  Serial.begin(115200);
//  Serial.setTimeout(250);
  
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  arduino.println("");
  arduino.print("AP IP address: ");
  arduino.println(myIP);

  if (MDNS.begin("esp8266")) {
    arduino.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/time", []()
  {

    request = "{";
    for (uint8_t i=0; i < server.args()-1; i++)
    {
      request += "'" + server.argName(i) + "':" + server.arg(i) + ",";
    }
    
    // GET request
    if(server.method() == HTTP_GET) 
    {
      request += "'method':0}";
    }


    // POST request
    if(server.method() == HTTP_POST) 
    {
      request += "\"method\":1}";
    }
    
    
    // create request send request
    arduino.print(request);

    // wait for response
    input = getResponse();

    // send response to client
    server.send(200, "text/plain", input);

    // reset strings
    input = "";
    request = "";

  });

  server.onNotFound(handleNotFound);

  server.begin();
  arduino.println("HTTP server started");
}


void loop(void){
  server.handleClient();
}

