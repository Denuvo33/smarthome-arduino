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
const char* apSSID = "SmartHome 33ZLT";
const char* apPassword = "connect-together";
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

  preferences.begin("WiFi", false);
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
    <!DOCTYPE html>
    <html>
    <head>
      <style>
        body {
          font-family: 'Trebuchet MS', Arial, sans-serif;
          background: radial-gradient(circle at 50% 50%, #1e3c72, #2a5298, #000);
          color: #ffffff;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
          overflow: hidden;
        }
        .background {
          position: absolute;
          width: 100%;
          height: 100%;
          top: 0;
          left: 0;
          z-index: -1;
          overflow: hidden;
        }
        .background span {
          position: absolute;
          display: block;
          width: 15px;
          height: 15px;
          background: radial-gradient(circle, #ffffff, rgba(255, 255, 255, 0));
          border-radius: 50%;
          animation: float 10s infinite ease-in-out;
        }
        .background span:nth-child(odd) {
          animation-duration: 12s;
          animation-delay: 2s;
        }
        @keyframes float {
          0% {
            transform: translateY(0) translateX(0) scale(1);
            opacity: 0.8;
          }
          50% {
            transform: translateY(-300px) translateX(150px) scale(0.5);
            opacity: 0.4;
          }
          100% {
            transform: translateY(-600px) translateX(-150px) scale(0.8);
            opacity: 0;
          }
        }
        .form-container {
          background: rgba(255, 255, 255, 0.1);
          padding: 20px;
          border-radius: 15px;
          backdrop-filter: blur(20px);
          box-shadow: 0 8px 32px rgba(31, 38, 135, 0.37);
          text-align: center;
          width: 350px;
        }
        .form-container h1 {
          margin-bottom: 20px;
          font-size: 28px;
          color: #ffffff;
        }
        .form-container input[type="text"],
        .form-container input[type="password"] {
          width: 100%;
          padding: 12px;
          margin: 10px 0;
          border: none;
          border-radius: 5px;
          background: rgba(255, 255, 255, 0.2);
          color: #ffffff;
          font-size: 16px;
        }
        .form-container input[type="text"]::placeholder,
        .form-container input[type="password"]::placeholder {
          color: #e0e0e0;
        }
        .form-container input[type="submit"] {
          background: #3163be;
          color: white;
          padding: 12px;
          border: none;
          border-radius: 5px;
          cursor: pointer;
          font-size: 16px;
          transition: background 0.3s, transform 0.3s;
        }
        .form-container input[type="submit"]:hover {
            background: #264172;
          transform: scale(1.05);
        }
        .title {
          background: rgba(45, 230, 255, 0.1);
          border-radius: 10px;
          padding: 10px;
          backdrop-filter: blur(15px);
          box-shadow: 0 8px 32px rgba(25, 0, 255, 0.25);
          color: #00ff80;
          font-size: 20px;
          font-weight: bold;
          margin-bottom: 20px;
        }
      </style>
    </head>
    <body>
      <div class="background">
        <span style="top: 20%; left: 10%;"></span>
        <span style="top: 40%; left: 70%;"></span>
        <span style="top: 80%; left: 30%;"></span>
        <span style="top: 60%; left: 90%;"></span>
        <span style="top: 10%; left: 50%;"></span>
        <span style="top: 70%; left: 20%;"></span>
      </div>
      <div class="form-container">
        <h1 class="title">Smart Home Created by Syahid with Laziness</h1>
        <form action="/save" method="POST">
          <input type="text" name="ssid" placeholder="Enter SSID" required><br>
          <input type="password" name="password" placeholder="Enter Password" required><br>
          <input type="submit" value="Save">
        </form>
      </div>
      </body>
      </html>
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
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <style>
        body {
          font-family: 'Trebuchet MS', Arial, sans-serif;
          background: radial-gradient(circle at 50% 50%, #1e3c72, #2a5298, #000);
          color: #ffffff;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
          overflow: hidden;
        }
        .title {
          background: rgba(45, 230, 255, 0.1);
          border-radius: 10px;
          padding: 10px;
          font-size: 20px;
          font-weight: bold;
        }
      </style>
    </head>
    <body>
      <h1 class="title">Smart Home Created by Syahid. Explore Everything and make it easy to control!</h1>
    </body>
    </html>
    )rawliteral"
    );
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
  delay(100);
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
    delay(100);
    long distance = ultrasonic.read(CM);
    server.send(200,"text/plain",String(distance));
  });

  server.on("/buzzer",[](){
    String state = (digitalRead(BUZZER) == HIGH) ? "1" : "0";
    server.send(200,"text/plain",state);
  });
  
  server.on("/buzzer/action",[](){
    int state = digitalRead(BUZZER);
    if(state == HIGH){
      digitalWrite(BUZZER,LOW);
      server.send(200,"text/plain","Turning on alarm");
    }else{
      digitalWrite(BUZZER,HIGH);
      server.send(200,"text/plain","Turning off alarm");
    }
  });
  server.begin();
}

void loop() {
 /* long distance = ultrasonic.read(CM);
  delay(100);  // Add a short delay to stabilize the sensor
  Serial.print("Distance: ");
  Serial.println(distance);
    if(distance < 15){
      digitalWrite(RED,HIGH);
      digitalWrite(BUZZER,HIGH);
    }else{
      digitalWrite(RED,LOW);
      digitalWrite(BUZZER,LOW);
    }*/
  server.handleClient();
}
