#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

#include "camera.h"

#include "time.h"

#define API_KEY "AIzaSyCjr5U8W62B2LGcdZiRl_A9FX5FEUzawRo"
#define FIREBASE_PROJECT_ID "segrebox"

#define USER_EMAIL "qwerty@gmail.com"
#define USER_PASSWORD "12345678"

#define STORAGE_BUCKET_ID "segrebox.appspot.com"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String TrashType;
boolean takeNewPhoto;


bool taskCompleted = false;

unsigned long dataMillis = 0;

String detectionResultString;


const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 3600;

String timeUpdated;

char formattedTime[25];

  // Define the interval for updating time in milliseconds
const unsigned long updateTimeInterval = 120000; // 2 minutes

// Variable to store the last time updateTimeFirestore was called
unsigned long lastUpdateTime = 0;

bool initTimeUpdate = true;

void updateTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Create a char array to store the formatted time
   // Adjust the size based on your formatting needs

  // Format the time and store it in the char array
  strftime(formattedTime, sizeof(formattedTime), "%d %B %Y, %H:%M", &timeinfo);

  // Print the formatted time
  Serial.println(formattedTime);

  // If you want to use the formatted time as a string elsewhere in your code, you can do so now.
  // For example, you can assign it to a String variable.
  // String myString = formattedTime;
  timeUpdated = formattedTime;


}


String getPhotoPath() {
  return String("/") + WiFi.macAddress() + ".jpg";
}

String getBucketPhoto() {
  return String("/waste-detections/") + WiFi.macAddress() + ".jpg";
}

void fcsUploadCallback(FCS_UploadStatusInfo info);

void capturePhotoSaveLittleFS(void) {
  // Dispose of the first pictures because of bad quality
  camera_fb_t* fb = NULL;
  // Skip the first 3 frames (increase/decrease the number as needed).
  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }

  // Turn on the flash
  pinMode(4, OUTPUT); // Built-in LED pin
  digitalWrite(4, HIGH); // Turn on the flash

  // Take a new photo
  fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  // Turn off the flash
  digitalWrite(4, LOW); // Turn off the flash
  pinMode(4, INPUT); // Release the LED pin

  // Photo file name
  String photoPath = getPhotoPath();
  Serial.printf("Picture file name: %s\n", photoPath.c_str());
  File file = LittleFS.open(photoPath.c_str(), FILE_WRITE);

  // Insert the data into the photo file
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.print("The picture has been saved in ");
    Serial.print(photoPath.c_str());
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  // Close the file
  file.close();
  esp_camera_fb_return(fb);
}

void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  } else {
    delay(500);
    Serial.println("LittleFS mounted successfully");
  }
}

// Function to write data to Firebase Firestore
void writeDataToFirebase() {

  updateTime();

  // Create a FirebaseJson object to set the data you want to write
  FirebaseJson content;

  content.set("fields/detail/stringValue", "...");
  content.set("fields/detection-result/stringValue", "plastic");
  content.set("fields/event/stringValue", "uuid event");
  content.set("fields/imageUrl/stringValue", "imageurl");
  content.set("fields/latitude/doubleValue", 1.0000);
  content.set("fields/longitude/doubleValue", 1.0000);
  content.set("fields/name/stringValue", "nama bin");
  content.set("fields/levelPlastic/doubleValue", 0.0);
  content.set("fields/levelPaper/doubleValue", 0.0);
  content.set("fields/levelOthers/doubleValue", 0.0);
  content.set("fields/timeUpdated/stringValue", timeUpdated);
  content.set("fields/objectDetected/booleanValue", 1);

  // Specify the document path
  String documentPath = "trash-bins/" + WiFi.macAddress();

  Serial.print("Creating document... ");

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
    Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}

// Function to get data from Firebase Firestore
void getDataFromFirebase() {
  String documentPath = "trash-bins/" + WiFi.macAddress();
  String mask = "`detection-result`";

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())) {
    Serial.println("Data fetched successfully.");

    // Parse the JSON data using ArduinoJson
    DynamicJsonDocument doc(2048);  // Adjust the size according to your data
    DeserializationError error = deserializeJson(doc, fbdo.payload());

    if (error) {
      Serial.print("Failed to parse JSON data: ");
      Serial.println(error.c_str());
    } else {

      detectionResultString = doc["fields"]["detection-result"]["stringValue"].as<String>();
      TrashType  = doc["fields"]["detection-result"]["stringValue"].as<String>();

      Serial.println("Detection result: ");
      Serial.print(detectionResultString);
    }
  } else {
    Serial.print("Failed to fetch data: ");
    Serial.println(fbdo.errorReason());
  }
}

void updateFirestoreFieldValue(const String& documentPath, const String& fieldPath, const String& newValue) {
  // Create a FirebaseJson object to hold the data you want to update
  FirebaseJson content;

  content.clear();

  // Set the new value for the field you want to update
  content.set(fieldPath.c_str(), newValue);

  String variablesUpdated = "name";

  Serial.print("Updating document... ");

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), variablesUpdated.c_str()))
    Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}

void shotAndSend() {
  if (takeNewPhoto) {
    capturePhotoSaveLittleFS();
    takeNewPhoto = false;
  }
  delay(1);
  if (Firebase.ready()) {
    Serial.print("Uploading picture... ");

    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID, getPhotoPath().c_str(), mem_storage_type_flash, getBucketPhoto().c_str(), "image/jpeg", fcsUploadCallback)) {
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
    } else {
      Serial.println(fbdo.errorReason());
    }
  }
  FirebaseJson content;

  content.set("fields/url/stringValue", "waste-detections/" + WiFi.macAddress() + ".jpg" );

  // Specify the document path
  String documentPath = "waste-detections/" + WiFi.macAddress();

  Serial.print("Creating photo document... ");

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw()))
    Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());

}

void updateTimeFirestore(){ //setelah gerakin motor panggil function ini di function tempat gerakin steppernya
  // Create a FirebaseJson object to hold the data you want to update
  updateTime();

  FirebaseJson content;

  String documentPath = "trash-bins/" + WiFi.macAddress();

  content.clear();

  // Set the new value for the field you want to update
  content.set("fields/timeUpdated/stringValue", timeUpdated);

  Serial.print("updating time");

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(),  "timeUpdated"))
    //Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
    Serial.println("ok");
  else
    Serial.println(fbdo.errorReason());
}

void resetObjectDetected(){ //setelah gerakin motor panggil function ini di function tempat gerakin steppernya

  FirebaseJson content;

  String documentPath = "trash-bins/" + WiFi.macAddress();
  Serial.println(WiFi.macAddress());

  content.clear();

  // Set the new value for the field you want to update
  content.set("fields/objectDetected/booleanValue", 0);

  Serial.print("resetting object detected");

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(),  "objectDetected"))
    //Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
    Serial.println("ok");
  else
    Serial.println(fbdo.errorReason());
}

void firebase_setup() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  initLittleFS();
  // Turn off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the API key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign-in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the callback function for the long-running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will be controlled by your code or third-party library, e.g., WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer needs to be set.
  // Large data transmission may require a larger RX buffer, otherwise connection issues or data read timeout can occur.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of the response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
}

void firebase_loop() {
  //if (Firebase.ready() && (millis() - dataMillis > 15000 || dataMillis == 0)) {
   // dataMillis = millis();
   
    if (!taskCompleted) {
      taskCompleted = true;
      writeDataToFirebase(); // Call the write data function
    }

    if (ObjectDetected == true){ //Trigger take pic 
      shotAndSend();
      takeNewPhoto = true;

      //Serial.println("firebase loop func called");
    }

    if (initTimeUpdate == true){
    updateTimeFirestore();
    initTimeUpdate = false;
   }

    // Check if 2 minutes have passed since the last updateTimeFirestore call
  if (millis() - lastUpdateTime >= updateTimeInterval) {
    lastUpdateTime = millis();
    updateTimeFirestore();
  }
    
    

    //getDataFromFirebase(); // Call the get data function

    //String documentPath = "trash-bins/" + WiFi.macAddress();
    //String fieldPath = "fields/name/stringValue";
    //String newValue = "asuuuuuu";

  //}
}

void fcsUploadCallback(FCS_UploadStatusInfo info) {
  if (info.status == firebase_fcs_upload_status_init) {
    Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
  } else if (info.status == firebase_fcs_upload_status_upload) {
    Serial.printf("Uploaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
  } else if (info.status == firebase_fcs_upload_status_complete) {
    Serial.println("Upload completed\n");
    FileMetaInfo meta = fbdo.metaData();

  } else if (info.status == firebase_fcs_upload_status_error) {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}