#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Wake Light";
const char* password = "stayinbed!";

String input = "";

ESP8266WebServer server(80);

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
  while(!Serial.available()) {} // wait for input
  while (Serial.available() > 0)
  {
    str = Serial.readString(); // Read serial
  }
  return str;
}


String setResponse(String request) 
{
  Serial.print(request);
  // wait for response
  return getResponse();
}

//String jsonRequest(ESP8266WebServer webServer) 
//{
//  Serial.println("Building JSON request from server object");
//  String json = "{";
//  for (uint8_t i=0; i < webServer.args()-1; i++)
//  {
//    json += "\"" + webServer.argName(i) + "\":" + webServer.arg(i) + ",";
//  }
//  json.setCharAt(json.length()-1, '}');
//  Serial.println(json);
//  return json;
//}

void setup(void){

  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  Serial.setTimeout(100);
  WiFi.softAP(ssid, password);
  Serial.println("");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/time", [](){
    
    if(server.method() == HTTP_GET) {
      input = getResponse();
      server.send(200, "text/plain", input);
    }

    if(server.method() == HTTP_POST) {
      // build json message with url params
      input = "{";
      for (uint8_t i=0; i < server.args()-1; i++)
      {
        input += "\"" + server.argName(i) + "\":" + server.arg(i) + ",";
      }
      input.setCharAt(input.length()-1, '}');
      Serial.print("Sending to client");
      Serial.println(input);
      server.send(200, "text/plain", input);
    }
    
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}


void loop(void){
  server.handleClient();
}

