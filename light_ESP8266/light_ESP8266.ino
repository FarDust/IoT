#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define LIGHT_PIN D4

WiFiServer server(80);
ESP8266WebServer WebServer(80);

StaticJsonBuffer<128> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

bool lightState = true;

void info(){
  WebServer.send(418, "application/json", "{\"type\": \"ligth\", \"methods\": \[\"status\",\"update\",\"toggle\"\]}");
  }


void ask(){
  String response;
  root["isOn"] = lightState;
  root.printTo(response);
  WebServer.send(100, "application/json", response);
  }

void lightManager() {
  lightState = !lightState;
  if (ledState) {
    root["isOn"] = true;
    digitalWrite(LIGHT_PIN, LOW);
  } else {
    root["isOn"] = false;
    digitalWrite(LIGHT_PIN, HIGH);
  }
  String response;
  root.printTo(response);
  WebServer.send(200, "application/json", response);
}

void control(){
  String rule = WebServer.arg("rule");
  Serial.println(rule);
  String response;
  if (rule == "on"){
    ledState = true;
    }else if(rule == "off"){
      ledState = false;
    }else{
      WebServer.send(400, "application/json", response);
      return ;
    }
  digitalWrite(LIGHT_PIN, !ledState);
  root["isOn"] = lightState;
  root.printTo(response);
  WebServer.send(200, "application/json", response);
  return ;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  WiFi.begin("<SSID>", "<password>");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.println("Waiting to connect...");
  }
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  WebServer.on("/", HTTP_GET, info);
  WebServer.on("/status", HTTP_GET, ask);
  WebServer.on("/update", HTTP_POST, control);
  WebServer.on("/toggle", HTTP_GET, lightManager);
  WebServer.begin();
}

void loop() {
  WebServer.handleClient();
}
