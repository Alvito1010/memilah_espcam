// #include <Arduino.h>
// #include <WiFi.h>
// #include <ArduinoJson.h>
// #include <Firebase_ESP_Client.h>
// #include <addons/TokenHelper.h>

// #include "camera.h"

// #define API_KEY "AIzaSyCjr5U8W62B2LGcdZiRl_A9FX5FEUzawRo"
// #define FIREBASE_PROJECT_ID "segrebox"

// #define USER_EMAIL "qwerty@gmail.com"
// #define USER_PASSWORD "12345678"

// #define STORAGE_BUCKET_ID "segrebox.appspot.com"

// // Define Firebase Data object
// FirebaseData fbdo;
// FirebaseAuth auth;
// FirebaseConfig config;

// String TrashType;
// boolean takeNewPhoto = true;

// int trashCategory;
// bool taskCompleted = false;

// unsigned long dataMillis = 0;

// //String detectionResultString;

// double levelPlastic = 0.111;//nanti gantiin pake data sensor
// double levelPaper = 0.522;
// double levelOthers = 0.133;

// double latCoordinates = 4.5882;//nanti gantiin pake data gps
// double longCoordinates = 9.153;

// String documentPath;



// void updateCoordinates(); //panggil di setup (msi blom bisa update valuenya)

// void updateLevels(); //Panggil abis get data dari sensor di loop

// // Function to get data from Firebase Firestore

// void getResult() { //panggil di loop bagian awal2
//   String documentPath = "trash-bins/A0:B7:65:5A:DA:44";
//   String mask = "`detection-result`";

//   if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())) {
//     Serial.println("Data fetched successfully.");

//     // Parse the JSON data using ArduinoJson
//     DynamicJsonDocument doc(2048);  // Adjust the size according to your data
//     DeserializationError error = deserializeJson(doc, fbdo.payload());

//     if (error) {
//       Serial.print("Failed to parse JSON data: ");
//       Serial.println(error.c_str());
//     } else {

//       //detectionResultString = doc["fields"]["'detection-result`"]["stringValue"].as<String>();
//       TrashType  = doc["fields"]["detection-result"]["stringValue"].as<String>();

//       if (TrashType == "Plastic") {
//         trashCategory = 0;
//       } else if (TrashType == "Paper") {
//         trashCategory = 1;
//       } else if (TrashType == "Others") {
//         trashCategory = 2;
//       }

//       Serial.println("Detection result: ");
//       Serial.print(TrashType);

//       Serial.println("int: ");
//       Serial.print(trashCategory);
//     }
//   } else {
//     Serial.print("Failed to fetch data: ");
//     Serial.println(fbdo.errorReason());
//   }
// }

// void resetDetectionResult(){ //setelah gerakin motor panggil function ini di function tempat gerakin steppernya
//   // Create a FirebaseJson object to hold the data you want to update
//   FirebaseJson content;

//   String documentPath = "trash-bins/A0:B7:65:5A:DA:44";

//   content.clear();

//   // Set the new value for the field you want to update
//   content.set("fields/detection-result/stringValue", "-");

//   Serial.print("resetting results... ");

//   if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(),  "`detection-result`"))
//     //Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
//     Serial.println("ok");
//   else
//     Serial.println(fbdo.errorReason());
// }

//  void updateFirestoreFieldValue(const String& documentPath, const String& fieldPath, const double& newValue, const String variablesUpdated) {// gausah dipanggil kemana2
//   // Create a FirebaseJson object to hold the data you want to update
//   FirebaseJson content;

//   content.clear();

//   // Set the new value for the field you want to update
//   content.set(fieldPath.c_str(), newValue);

//   if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), variablesUpdated.c_str()))
//     Serial.printf("OK\n%s\n\n", fbdo.payload().c_str());
//   else
//     Serial.println(fbdo.errorReason());
//   }

// void firebase_setup() { // panggil di setup

//   Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

//   /* Assign the API key (required) */
//   config.api_key = API_KEY;

//   /* Assign the user sign-in credentials */
//   auth.user.email = USER_EMAIL;
//   auth.user.password = USER_PASSWORD;

//   /* Assign the callback function for the long-running token generation task */
//   config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

//   // Comment or pass false value when WiFi reconnection will be controlled by your code or third-party library, e.g., WiFiManager
//   Firebase.reconnectNetwork(true);

//   // Since v4.4.x, BearSSL engine was used, the SSL buffer needs to be set.
//   // Large data transmission may require a larger RX buffer, otherwise connection issues or data read timeout can occur.
//   fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

//   // Limit the size of the response payload to be collected in FirebaseData
//   fbdo.setResponseSize(2048);

//   Firebase.begin(&config, &auth);
//   Firebase.reconnectWiFi(true);

//   updateCoordinates(); //nanti taro sini
  
// }

// void firebase_loop() { // panggil di loop
//   if (Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0)) {
//     dataMillis = millis();   

//     updateLevels();

//     getResult();

    

//     //resetDetectionResult(); abis fetch get result dan udah gerakin motor, panggil ini func
    
//   }
// }

// void updateLevels() { //panggil di loop
//   String documentPath = "trash-bins/A0:B7:65:5A:DA:44";
//   String fieldPathPlastic = "fields/fillPlastic/doubleValue";
//   String fieldPathPaper = "fields/fillPaper/doubleValue";
//   String fieldPathOthers = "fields/fillOthers/doubleValue";
//   String variableUpdatedPlastic = "fillPlastic";
//   String variableUpdatedPaper = "fillPaper";
//   String variableUpdatedOthers = "fillOthers";

//   Serial.println("Updating levels...");

//    updateFirestoreFieldValue(documentPath, fieldPathPlastic, levelPlastic, variableUpdatedPlastic);
//    updateFirestoreFieldValue(documentPath, fieldPathPaper, levelPaper, variableUpdatedPaper);
//    updateFirestoreFieldValue(documentPath, fieldPathOthers, levelOthers, variableUpdatedOthers);
// }

// void updateCoordinates(){ //panggil di setup
//   String documentPath = "trash-bins/A0:B7:65:5A:DA:44";
//   String fieldlatitude = "fields/latitude/doubleValue";
//   String fieldlongitude = "fields/longitude/doubleValue";
//   String variablesUpdatedLat = "latitude";
//   String variablesUpdatedLong = "longitude";

//   // Create a FirebaseJson object to hold the data you want to update
//   FirebaseJson content;

//   content.clear();

//   // Set the new value for the field you want to update
//   content.set(fieldlatitude.c_str(), latCoordinates);
//   content.set(fieldlongitude.c_str(), longCoordinates);

  

//   Serial.print("Updating coordinates... ");

//   updateFirestoreFieldValue(documentPath, fieldlatitude, latCoordinates, variablesUpdatedLat);
//   updateFirestoreFieldValue(documentPath, fieldlongitude, longCoordinates, variablesUpdatedLong);
// }