#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "Motor/Motor.h"

// TODO: Inject const values into board somehow at build time.
// TODO: How can we persist the fact that the vent has been registered?

/*
  Vent Properties
  Each vent board will be flashed with a unique serial/code pair used to authenticate when
  registering itself upon connecting to the server once it has received WiFi creds from
  mobile app.
*/
const String VENT_SERIAL = "dev-vent";
const String VENT_CODE = "123456";
const String VENT_CREDS = "{\"serial\": \"" + VENT_SERIAL + "\",\"code\": \"" + VENT_CODE + "\"}";

/* WiFi credentials */
const char* AP_SSID = "IntelliVent";
const char* AP_PWD = "Int3ll1V3nt!";

/* API */
const std::string BASE_URL = "http://192.168.1.154:3000/api/v1/";

ESP8266WebServer server(80);
bool connectedToInternet = false;
bool isMoving = false;

// TODO: Accept userId and POST it along to /register endpoint
// so that the vent will be linked to a user and viewable in
// the mobile app.
void registerVent(/*const char* userId*/)
{
    HTTPClient http; //Declare an object of class HTTPClient

    http.begin((BASE_URL + "vents/register").c_str()); //Specify request destination
    http.addHeader("Content-Type", "application/json");
    Serial.println(VENT_CREDS);
    int httpCode = http.POST(VENT_CREDS); //Send the request
    if (httpCode == 200 || httpCode == 201) { // Check the returning code
      connectedToInternet = true;
      String payload = http.getString(); //Get the request response payload
      Serial.println(payload); //Print the response payload
    }
    else if (httpCode > -1) {
      String payload = http.getString(); //Get the request response payload
      Serial.println(payload); //Print the response payload
    }
    else {
      Serial.print("httpCode = ");
      Serial.println(httpCode);
    }

    http.end(); //Close connection
}

void handleConnect()
{
  if (server.method() != HTTP_POST) {
    Serial.println("HTTP request method not supported.");
    return;
  }
  // if (!server.hasHeader("Content-Type") || server.header("Content-Type") != "application/json") {
  //   Serial.println("'Content-Type' header is missing or not supported.");
  //   return;
  // }
  if(!server.hasArg("ssid") || !server.hasArg("pwd")) {
    Serial.println("Missing required args.");
    return;
  }

  // Returning a 200 with empty JSON prior to attempting connection
  server.send(200, "application/json", "{}");

  // Try connecting to WiFi network with creds provided
  String wifissid = server.arg("ssid");
  String wifipwd = server.arg("pwd");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid, wifipwd);

  int tryCount = 0;
  while (WiFi.status() != WL_CONNECTED || tryCount >= 10) {
    delay(1000);
    tryCount++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected! Registering vent with server.");
    registerVent();
    server.stop();
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PWD);
  }
}

void setup()
{
  delay(1000); // Helps ensure Serial.print statements work
  Serial.begin(9600);

  // Configure motor control pins to be output and turned off
  int control_pins_len = (sizeof(MOTOR_CONTROL_PINS) / sizeof(MOTOR_CONTROL_PINS[0]));
  for (int x = 0; x < control_pins_len; x++)
  {
    pinMode(MOTOR_CONTROL_PINS[x], OUTPUT);
    digitalWrite(MOTOR_CONTROL_PINS[x], LOW);
  }

  // Start a WiFi Access Point for initial configuration
  // Docs: https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html#set-up-network
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PWD);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/connect", handleConnect);
  server.begin();
}

void loop()
{
  // Handle client requests until connected to home WiFi
  if (!connectedToInternet)
  {
    server.handleClient();
  }
  else if (isMoving)
  {
    Serial.println("isMoving...");
    return;
  }
  else
  {
    Serial.println("Delaying for 5 seconds");
    delay(5000);
    Serial.println("Retrieving jobs to process");

    HTTPClient http;

    http.begin((BASE_URL + "jobs/process").c_str());
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(VENT_CREDS);
    if (httpCode == 200) {
      String payload = http.getString();
      http.end();
      Serial.println(payload);

      int openIndex = payload.indexOf("open");
      int closeIndex = payload.indexOf("close");

      if (openIndex > -1 && closeIndex > -1)
      {
        if (openIndex > closeIndex)
        {
          Serial.println("Both, opening first");
          isMoving = true;
          Motor::rotateMotor(false, 180);
          delay(500);
          Motor::rotateMotor(true, 180);
          isMoving = false;
        }
        else
        {
          Serial.println("Both, closing first");
          isMoving = true;
          Motor::rotateMotor(true, 180);
          delay(500);
          Motor::rotateMotor(false, 180);
          isMoving = false;
        }
      }
      else if (openIndex > -1 && closeIndex == -1)
      {
        Serial.println("Open, no close");
        isMoving = true;
        Motor::rotateMotor(false, 180);
        delay(500);
        isMoving = false;
      }
      else if (closeIndex > -1 && openIndex == -1)
      {
        Serial.println("Close, no open");
        isMoving = true;
        Motor::rotateMotor(true, 180);
        delay(500);
        isMoving = false;
      }
      else
      {
        Serial.println("No jobs to process");
      }
    }
    else if (httpCode > -1) {
      String payload = http.getString(); //Get the request response payload
      http.end();
      Serial.println(payload); //Print the response payload
    }
    else
    {
      Serial.print("httpCode = ");
      Serial.println(httpCode);
    }
  }
}
