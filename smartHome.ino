#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Ultrasonic.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define TRIG_PIN 4
#define ECHO_PIN 5
#define BUZZER 12
#define DHTPIN 16
#define DHTTYPE DHT11 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Ultrasonic ultrasonic(TRIG_PIN,ECHO_PIN);
DHT dht(DHTPIN,DHTTYPE);


#define LED 2
#define RED 13

WebServer server(80);
Preferences preferences;

// Variabel Wi-Fi
const char* apSSID = "ESP32-Config";
const char* apPassword = "12345678";
char wifiSSID[32];
char wifiPassword[32];

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }
  //Speaker
  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,LOW);
  //LED
  pinMode(LED,OUTPUT);
  pinMode(RED,OUTPUT);
  digitalWrite(LED,LOW);
  digitalWrite(LED,LOW);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  preferences.begin("wifi", false);
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");

  if (savedSSID != "" && savedPassword != "") {
    connectToWiFi(savedSSID.c_str(), savedPassword.c_str());
  } else {
    startAccessPoint();
  }
}

void connectToWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  display.println("Connecting to Wi-Fi...");
  display.display();
  Serial.println("Connecting to Wi-Fi...");

  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    Serial.println("Connected to Wi-Fi");
    Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.println("Connected to Wi-Fi");
    display.println(WiFi.localIP());
    display.display();

    if (!MDNS.begin("esp32-device")) {
      Serial.println("Error starting mDNS");
      return;
    }
    MDNS.addService("http", "tcp", 80);
    setupServer();
  } else {
    Serial.println("Failed to connect. Starting AP...");
    startAccessPoint();
  }
}

void startAccessPoint() {
  WiFi.softAP(apSSID, apPassword);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP Address: ");
  Serial.println(IP);

  display.clearDisplay();
  display.println("AP Mode Enabled");
  display.println("Connect to:");
  display.println(apSSID);
  display.display();

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", R"rawliteral(
      <form action="/save" method="POST">
        SSID: <input type="text" name="ssid"><br>
        Password: <input type="text" name="password"><br>
        <input type="submit" value="Save">
      </form>
    )rawliteral");
  });

  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid != "" && password != "") {
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      server.send(200, "text/html", "Wi-Fi saved! Restarting...");
      delay(2000);
      ESP.restart();
    } else {
      server.send(400, "text/html", "Invalid input!");
    }
  });

  server.begin();
}

void setupServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/plain", "Welcome To Smart Home");
  });

  server.on("/led/on",[]() {
      digitalWrite(LED, HIGH);
      server.send(200, "text/plain", "LED is ON");
  });
  server.on("/led/off",[]() {
      digitalWrite(LED, LOW);
      server.send(200, "text/plain", "LED is OFF");
  });
  server.on("/led",[](){
    int state = digitalRead(LED);
    String response = (state == HIGH ) ? "1" : "0";
    server.send(200,"text/plain", response);
  });
  server.on("/dht", []() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    server.send(500, "text/plain", "Failed to read DHT sensor");
  } else {
    String jsonResponse = "{";
    jsonResponse += "\"temp\": " + String(temp) + ",";
    jsonResponse += "\"hum\": " + String(hum);
    jsonResponse += "}";

    server.send(200, "application/json", jsonResponse);
  }
});

  server.on("/ultrasonic",[](){
    long distance = ultrasonic.read(CM);
    server.send(200,"text/plain",String(distance));
  });
  server.begin();
}

void loop() {
  long distance = ultrasonic.read(CM);
  delay(100);  // Add a short delay to stabilize the sensor
  Serial.print("Distance: ");
  Serial.println(distance);
    if(distance < 15){
      digitalWrite(RED,HIGH);
      digitalWrite(BUZZER,HIGH);
    }else{
      digitalWrite(RED,LOW);
      digitalWrite(BUZZER,LOW);
    }
  server.handleClient();
}
