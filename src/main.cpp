#include <Wire.h>
#include <Arduino.h>

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "infrared.h"
#include "firebase_functions.h"
#include "firebase_trial.h"

#include "wifi_setup.h"

#include <Update.h>
#include <WebServer.h>
#include <DNSServer.h>


//Infrared variable
bool ObjectDetected;

//Built in LED FLASH
#define FLASH_GPIO_NUM 4


static void MoveStepper(void *pvParameters);

void setup() {
  
  Serial.begin(115200);
  WiFiManager wm;

  bool res;
  res = wm.autoConnect("AutoConnectAP", "password"); // Set your SSID and password

  if (!res) {
    Serial.println("Failed to connect");
    // ESP.restart();
  } else {
    Serial.println("Connected...yeey :)");
  }

  //WiFi Settings
  //WiFiConnect();

  //Firebase Settings
  firebase_setup();

  pinMode(FLASH_GPIO_NUM, OUTPUT);


  //Infrared Settings
  pinMode(SENSOR_PIN, INPUT);

  //Built in LED
  // pinMode(33, OUTPUT);

  //xTaskCreatePinnedToCore(MoveStepper, "MoveStepper", 2048, NULL, 2, NULL, 0); 
  
}

void loop() {

  //vTaskDelay(0);

  // if(ObjectDetected == true){
  //   delay(1000);
  //   firebase_loop();
  // }
  InfraredSensor();

   if(ObjectDetected == true){
    Serial.println("detected");
  }

  firebase_loop();

  //InfraredSensor();

  // if(ObjectDetected == true){
  //     digitalWrite(FLASH_GPIO_NUM, HIGH);
  //     Serial.println("Object detected");
  //     delay(2000);
  //   }else{
  //     digitalWrite(FLASH_GPIO_NUM, LOW);
  //     Serial.println("Object not detected");
  //     delay(2000);
  //   }
    
 

 }

void MoveStepper(void *pvParameters) {

  while (1){

    
    vTaskDelay(10);

    
  }
}




