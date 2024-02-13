#include "wifi_setup.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager


void WiFiConnect(){
WiFi.mode(WIFI_AP_STA);
  WiFi.begin("iPhone vito", "12345678");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}