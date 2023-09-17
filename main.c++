#include <WiFi.h>
#include <PubSubClient.h>
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {0, A1, 1, A0};
byte colPins[COLS] = {2, 3, 4, 5};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Define your Wi-Fi credentials
const char* SSID = "ANDAR SUPERIOR";
const char* PASSWORD = "baba404040";

// MQTT configuration
const char* BROKER_MQTT = "192.168.0.80";
int BROKER_PORT = 1883;
const char* TOPICO_SUBSCRIBE = "/lock/control"; // MQTT topic for lock control
const char* TOPICO_PUBLISH = "/lock/status";    // MQTT topic for lock status

// Create a WiFi client and MQTT client
WiFiClient espClient;
PubSubClient MQTT(espClient);

// OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PIN code for the lock
char code[] = {'1', '2', '3', '4'}; //Seu código de acesso
const int codeSize = sizeof(code) / sizeof(code[0]);
char inputCode[codeSize];
int inputIndex = 0;

void setup() {
  // Initialize the OLED display
  display.begin(SSD1306_I2C_ADDRESS, SSD1306_I2C_SDA, SSD1306_I2C_SCL);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Initialize serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
  reconnectMQTT();

  // Display initial message
  displayMessage("Enter PIN:");
}

void loop() {
  char key = keypad.getKey();
  if (key != NO_KEY) {
    inputCode[inputIndex] = key;
    display.setCursor(inputIndex * 8, 24);
    display.print("*");
    inputIndex++;

    if (inputIndex == codeSize) {
      checkPIN();
    }
  }
  MQTT.loop();
}

void displayMessage(const char* message) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}

void checkPIN() {
  bool isCorrectCode = true;
  for (int i = 0; i < codeSize; i++) {
    if (inputCode[i] != code[i]) {
      isCorrectCode = false;
      break;
    }
  }

  if (isCorrectCode) {
    displayMessage("Access granted!");
    MQTT.publish(TOPICO_PUBLISH, "unlocked");
  } else {
    displayMessage("Access denied!");
    MQTT.publish(TOPICO_PUBLISH, "denied");
  }

  delay(2000);
  resetInput();
  displayMessage("Enter PIN:");
}

void resetInput() {
  inputIndex = 0;
  memset(inputCode, 0, codeSize);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Handle MQTT messages if needed
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (MQTT.connect("ESP32Client")) {
      Serial.println("Connected to MQTT broker");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Failed to connect to MQTT broker, retrying in 2 seconds...");
      delay(2000);
    }
  }
}
