#include "BluetoothSerial.h"
#include <WiFi.h>
#include <WebSocketClient.h>
#include <vector>

struct MOTOR_PINS
{ 
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {15, 13}, //RIGHT_MOTOR Pins (EnA, IN1, IN2)
  {14, 2},  //LEFT_MOTOR  Pins (EnB, IN3, IN4)
};
#define LIGHT_PIN 4

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1

String device_name = "ESP32-TRI";
const char* ssid = "";
const char* password = "";
String get_info[3];
int j = 0;
String info;
char path[] = "/order";
const char* host = "";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

bool bluetooth_disconnect = false;
long start_wifi_millis;
long wifi_timeout = 10000;

enum connection_stage { PAIRING, WIFI, WEBSOCKET, CONNECTED };
enum connection_stage stage = PAIRING;

WebSocketClient webSocketClient;
BluetoothSerial SerialBT;
WiFiClient client;

void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);     
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}

void moveCar(String command)
{
  if (command == "FW")
    {
      digitalWrite(LIGHT_PIN, HIGH);
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);
      digitalWrite(LIGHT_PIN, LOW);
    }
  else if (command == "ST")
    {
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
    }
  else if (command == "RL")
    {
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);
    }
  else if (command == "RR")
    {
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);
    }
  else if (command == "BW")
    {
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);
    }
  else
    {
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);
    }
}

void setUpPinModes()
{
  for (int i = 0; i < motorPins.size(); i++)
  {    
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  
  }
  pinMode(LIGHT_PIN, OUTPUT);
  moveCar("ST");
}

void setup() {
  setUpPinModes();
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
}

void disconnect_bluetooth()
{
  delay(1000);
  Serial.println("BT stopping");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("BT stopped");
  delay(1000);
  bluetooth_disconnect = false;
}

bool init_wifi()
{
  Serial.println(ssid);
  Serial.println(password);
  start_wifi_millis = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi_millis > wifi_timeout)
      {
        WiFi.disconnect(true, true);
        return false;
      }
    }
  return true;
}

void loop() {
  delay(20);
  switch (stage)
  {
    case PAIRING:
      Serial.println("Waiting for SSID provided by server... ");
      Serial.println("Waiting for password provided by server... ");
      while (info == "")
      {
        if (SerialBT.available()) {
        info = SerialBT.readString();
        }
      }
      for (auto x : info)
      {
        if (x == '|')
        {
          j++;
          continue;  
        }
        else
          get_info[j] = get_info[j] + x;
      }
      ssid = get_info[0].c_str();
      password = get_info[1].c_str();
      host = get_info[2].c_str();
      info = "";
      stage = WIFI;
      break;

    case WIFI:
      Serial.println("Waiting for Wi-Fi connection");
      if (init_wifi())
      {
        Serial.println("");
        Serial.println("Wifi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        disconnect_bluetooth();
        stage = WEBSOCKET;
      }
      else
      {
        Serial.println("Wi-Fi connection failed");
        delay(2000);
        stage = PAIRING;  
      }
      break;

    case WEBSOCKET:
      Serial.println(host);
      if (client.connect(host, 5000))
      {
        Serial.println("Webserver connected");
      }
      else
      {
        Serial.println("Webserver connection failed");
        break;
      }
      webSocketClient.path = path;
      webSocketClient.host = const_cast<char*>(host);
      if (webSocketClient.handshake(client))
      {
        Serial.println("Handshake successful");
        stage = CONNECTED;
      }
      else
      {
        Serial.println("Handshake failed");
        break;
      }
      break;

    case CONNECTED:
      if (client.connected())
      {
        String data;
        webSocketClient.getData(data);
        if (data.length() > 0)
        {
        Serial.print("Received data: ");
        Serial.println(data);
        moveCar(data);
        }
      }
      else
      {
        Serial.println("Client disconnected");
        stage = PAIRING;
      }
      break;
  }
}
