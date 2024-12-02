#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Fakepng-IoT"; //อย่าลืมแก้ไข SSID ของ WIFI ที่จะให้ NodeMCU ไปเชื่อมต่อ
const char* password = "iotengineering"; //อย่าลืมแก้ไข PASSWORD ของ WIFI
const char* mqtt_server = "project.local"; //อย่าลืมแก้ไข BROKER
const int mqtt_port = 1883;
const char* mqtt_Client = "LAB";  //อย่าลืมแก้ไข ClientID
const char* mqtt_username = "ham";  //อย่าลืมแก้ไข Token
const char* mqtt_password = "ham"; //อย่าลืมแก้ไข Secret

WiFiClient espClient; 
PubSubClient client(espClient);

const int pingPin1 = 2;
const int inPin1 = 3;
const int pingPin2 = 4;
const int inPin2 = 5;
const int pingPin3 = 7;
const int inPin3 = 8;
const int pingPin4 = 10;
const int inPin4 = 11;

long lastMsg = 0;
int value = 0;
char msg[100];
String DataString;
int allTable = 4;  // Total number of tables
int cm1, cm2, cm3, cm4;
int status1, status2, status3, status4;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
  if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) { //เชื่อมต่อกับ MQTT BROKER
    Serial.println("connected");
    client.subscribe("@msg/operator");
  }
  else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println("try again in 5 seconds");
    delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(pingPin1, OUTPUT);
  pinMode(inPin1, INPUT);
  pinMode(pingPin2, OUTPUT);
  pinMode(inPin2, INPUT);
  pinMode(pingPin3, OUTPUT);
  pinMode(inPin3, INPUT);
  pinMode(pingPin4, OUTPUT);
  pinMode(inPin4, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //เชื่อมต่อกับ WIFI
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //เชื่อมต่อกับ WIFI สำเร็จ แสดง IP
  client.setServer(mqtt_server, mqtt_port); //กำหนด MQTT BROKER, PORT ที่ใช้
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 3000) {  // Update every 3 seconds
    lastMsg = now;
    update();
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected to WiFi with IP: ");
  Serial.println(WiFi.localIP());
}

void update() {
    // Read status and distance at the same time
    detectAndUpdate(pingPin1, inPin1, status1, cm1);
    detectAndUpdate(pingPin2, inPin2, status2, cm2);
    detectAndUpdate(pingPin3, inPin3, status3, cm3);
    detectAndUpdate(pingPin4, inPin4, status4, cm4);

    Serial.println("Chair one     : " + (String)status1 + " people");
    Serial.println("Chair two     : " + (String)status2 + " people");
    Serial.println("Chair three   : " + (String)status3 + " people");
    Serial.println("Chair four    : " + (String)status4 + " people");
    Serial.println("---------------------------");
    Serial.println("Chair one     : " + (String)cm1 + "     cm");
    Serial.println("Chair two     : " + (String)cm2 + "     cm");
    Serial.println("Chair three   : " + (String)cm3 + "     cm");
    Serial.println("Chair four    : " + (String)cm4 + "     cm");

    int Numpeople1 = status1 + status2 + status3 + status4;

    int tableStatus1;

    if (Numpeople1 > 0) {
    tableStatus1 = 0;
    Serial.println("tableStatus1: " + String(tableStatus1));
    Serial.println("Tables are occupied");
  } else {
    tableStatus1 = 1;
    Serial.println("tableStatus1: " + String(tableStatus1));
    Serial.println("Tables are empty");
  }


    String statusMessage = "{\"status\": " + String(tableStatus1) + ", \"Numpeople\": " + String(Numpeople1) + "}";

    statusMessage.toCharArray(msg,100);
    // Publish status to MQTT
    client.publish("data01/people01/table01", msg);

    Serial.println("Published: " + statusMessage);

    Serial.println("==========================");
}

void detectAndUpdate(int pingPin, int inPin, int &status, int &cm) {
  long duration;

  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  duration = pulseIn(inPin, HIGH);
  cm = microsecondsToCentimeters(duration);

  if (cm < 17) {
    digitalWrite(LED_BUILTIN, 0);
    status = 1;  // Occupied
  } else {
    digitalWrite(LED_BUILTIN, 1);
    status = 0;  // Empty
  }
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}
