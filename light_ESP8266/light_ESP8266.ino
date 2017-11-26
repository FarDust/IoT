#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define LIGHT_PIN 5

WiFiServer server(80);
ESP8266WebServer WebServer(80);

StaticJsonBuffer<128> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

bool lightState = false;
const char* ssid= "<SSID>";
const char* password= "<WiFiPassword>";
const char* OTApassword = "<OtaPassword>";

void info() {
  WebServer.send(200, "application/json", "{\"type\": \"ligth\", \"methods\": \[\"status\", \"update\", \"toggle\"\]}");
}


void ask() {
  String response;
  root["isOn"] = !lightState;
  root.printTo(response);
  WebServer.send(200, "application/json", response);
}

void lightManager() {
  lightState = !lightState;
  if (lightState) {
    root["isOn"] = false;
    digitalWrite(LIGHT_PIN, LOW);
  } else {
    root["isOn"] = true;
    digitalWrite(LIGHT_PIN, HIGH);
  }
  String response;
  root.printTo(response);
  WebServer.send(200, "application/json", response);
}

void control() {
  String rule = WebServer.arg("rule");
  Serial.println(rule);
  String response;
  if (rule == "on") {
    lightState = true;
  } else if (rule == "off") {
    lightState = false;
  } else {
    WebServer.send(400, "application/json", response);
    return ;
  }
  digitalWrite(LIGHT_PIN, lightState);
  root["isOn"] = lightState;
  root.printTo(response);
  WebServer.send(200, "application/json", response);
  return ;
}

void setup() {
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, !lightState);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect...");
  }
  Serial.println(WiFi.localIP());
  if (MDNS.begin("light1", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  ArduinoOTA.setPassword(OTApassword);
  WebServer.on("/", HTTP_GET, info);
  WebServer.on("/status", HTTP_GET, ask);
  WebServer.on("/update", HTTP_POST, control);
  WebServer.on("/toggle", HTTP_GET, lightManager);
  WebServer.begin();
  ArduinoOTA.begin();
}

void loop() {
  WebServer.handleClient();
  ArduinoOTA.handle();
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart();
  }
}
