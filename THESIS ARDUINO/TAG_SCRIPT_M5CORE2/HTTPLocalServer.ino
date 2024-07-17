// WIFI Settings
const char* WIFI_SSID  = "FaroukhGSM";
const char* WIFI_PASSWORD  = "12345678";
const char* TIME_SERVER_URL  = "http://192.168.183.233:3000/current-time";
const char* DISTANCE_SERVER_URL  = "http://192.168.183.233:3000/distance";

void initWifi(){
    // Connect to WiFi
    WiFi.begin(WIFI_SSID , WIFI_PASSWORD );
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Connecting to WiFi...");
      delay(2000);
    }
    Serial.println("Connected to WiFi");
}

// Send relative positions to server
void sendDataToServer(String tagId, String dA, String dB, String dC) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(distanceServerUrl);
    http.addHeader("Content-Type", "application/json");

    // Create JSON object
    StaticJsonDocument<200> doc;
    doc["tagId"] = tagId;
    doc["dA"] = dA;
    doc["dB"] = dB;
    doc["dC"] = dC;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.println("Sending payload: " + jsonPayload); // Debug: print the payload

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}