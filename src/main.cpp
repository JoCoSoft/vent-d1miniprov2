#include <ESP8266WebServer.h>
#include "Motor/Motor.h"

/* WiFi credentials */
const char ssid[] = "YOUR_SSID_HERE";
const char password[] = "YOUR_PASSWORD_HERE";

ESP8266WebServer server(80);
bool connected = false;
bool isMoving = false;

void handleMove()
{
  // Tell client the move will be handled
  server.send(200, "application/json", "{}");

  // Return early and don't rotate if already moving or if there's no name arg
  if (isMoving)
  {
    Serial.println("I'm already moving!");
    return;
  }
  if (!server.hasArg("name"))
  {
    Serial.println("No 'name' arg provided.");
    return;
  }

  // Rotate motor clockwise for "close" and counter-clockwise for "open"
  Serial.println(server.arg("name"));
  isMoving = true;
  Motor::rotateMotor(server.arg("name") == "close", 180);
  isMoving = false;
}

void setup()
{
  Serial.begin(9600);

  // Configure pins to be output and turned off
  int control_pins_len = (sizeof(MOTOR_CONTROL_PINS) / sizeof(MOTOR_CONTROL_PINS[0]));
  for (int x = 0; x < control_pins_len; x++)
  {
    pinMode(MOTOR_CONTROL_PINS[x], OUTPUT);
    digitalWrite(MOTOR_CONTROL_PINS[x], LOW);
  }

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int tryCount = 0;
  while (WiFi.status() != WL_CONNECTED || tryCount >= 10)
  {
    Serial.print("Trying to connect to ");
    Serial.println(ssid);
    delay(1000);
    tryCount++;

    connected = WiFi.status() == WL_CONNECTED;
    if (connected)
    {
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
  }

  if (!connected)
  {
    Serial.println("Failed to connect to WiFi. Not starting web server.");
    WiFi.mode(WIFI_OFF);
    return;
  }

  // Start web server and listen for requests to the /move endpoint
  server.on("/move", handleMove);
  server.begin();
}

void loop()
{
  if (connected)
  {
    // Handle client requests to erver
    server.handleClient();
  }
}
