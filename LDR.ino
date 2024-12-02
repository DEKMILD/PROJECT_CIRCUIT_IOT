#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6UmnO19yV"
#define BLYNK_TEMPLATE_NAME "IoTCircuit"
#define BLYNK_AUTH_TOKEN "6U6b7VYs_NCPAW3QHoeRcB1gaSEHYjad"

#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>  // Library for handling JSON
#include <BlynkSimpleWifi.h>

// Setting up LCD I2C (0x27 is the LCD address)
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long previousMillis = 0;       // ตัวแปรสำหรับเก็บเวลาที่ผ่านมา
const long interval = 2000;            // ตั้งเวลา 2 วิ
bool showVacantTable = true;            // ตัวแปรสำหรับสลับข้อความ


// WiFi Settings
const char* ssid = "Fakepng-IoT";
const char* password = "iotengineering";

//Blynk
char auth[] = "6U6b7VYs_NCPAW3QHoeRcB1gaSEHYjad";  // Replace with your Blynk auth token


// MQTT Settings
const char* mqtt_server = "project.local";  // Example broker
const char* topic = "out/LCD";  // MQTT topic you are subscribing to
const int mqtt_port = 1883;
const char* mqtt_Client = "Lcd";  // Unique ClientID
const char* mqtt_username = "ham";  // MQTT username (if any)
const char* mqtt_password = "ham";  // MQTT password (if any)

WiFiClient espClient;
PubSubClient client(espClient);

// Connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("WiFi connected! IP address: ");
  Serial.println(WiFi.localIP());  // Print IP address for debugging
}

// MQTT callback function when a message is received
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("Message received: " + message);
  
  // Parse JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  

 
  if (error) {
     Serial.println("Failed to parse JSON");
     return;
   }
  
   // Extract values from JSON
   int alltable = doc["alltable"];
   int allpeople = doc["allpeople"];
   int seatleft = 8-allpeople;

    Blynk.virtualWrite(V5, seatleft);

    unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // รีเซ็ตเวลาให้เท่ากับเวลาปัจจุบัน

    // เคลียร์หน้าจอแล้วเลือกแสดงข้อความตามเงื่อนไข
    lcd.clear();
    if (showVacantTable) {
      // แสดงข้อความ "Vacant table" และจำนวนโต๊ะ
      lcd.setCursor(0, 0);
      lcd.print("Vacant table: ");
        // แสดงผลในบรรทัดที่ 2
      lcd.print(alltable);
      lcd.setCursor(0, 1); 
      lcd.print("People: ");
      lcd.print(allpeople);
        // แสดงผลในบรรทัดที่ 2

      Serial.println("Vacant table: " + String(alltable));
    } else {
      // แสดงข้อความ "People" และจำนวนคน
      lcd.setCursor(0, 0);
      lcd.print("Vacant seat: ");
      //lcd.setCursor(0, 1);   // แสดงผลในบรรทัดที่ 2
      lcd.print(seatleft);

      Serial.println("Vacant seat: " + String(seatleft));
      Serial.println("People: " + String(allpeople));
    }

    // สลับข้อความในรอบถัดไป
    showVacantTable = !showVacantTable;
  
//   // Display data on the LCD
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Vacant table: ");
//   lcd.print(alltable);
//   //Serial.println(alltable);
  
//   lcd.setCursor(0, 1);
//   lcd.print("People: ");
//   lcd.print(allpeople);
//   //Serial.println(allpeople);
//   Serial.println(seatleft);
 }
}

// Reconnect to MQTT if disconnected
void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
  if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT");
      client.subscribe(topic);  // Subscribe to your topic
    } else {
      Serial.print("Failed to connect to MQTT, rc=");
      Serial.print(client.state());  // Print the error code
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

void setup() {
  Serial.begin(115200);
  lcd.begin();  // Initialize LCD without parameters
  lcd.backlight();
  setup_wifi();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Process MQTT messages
  Blynk.run();  // Run Blynk

}


