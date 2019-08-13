#include <ESP8266WiFi.h>
#include "FS.h"

WiFiServer server(80);

const String speedString = "speed=";
const int speedStringLength = speedString.length();
const String directionString = "direction=";
const int directionStringLength = directionString.length();

const int leftEnablePin = D0;
const int leftForwardPin = D1;
const int leftBackwardPin = D2;

const int rightEnablePin = D4;
const int rightForwardPin = D3;
const int rightBackwardPin = D5;

const int FORWARDS = 1;
const int STOP = 0;
const int BACKWARDS = -1;


int speed = 0;
int direction = 0;

void setup() {
  SPIFFS.begin();

  Serial.begin(115200);
  Serial.print("Waiting for client ");

  setupWiFi();
  server.begin();

  pinMode(rightEnablePin, OUTPUT);
  pinMode(rightForwardPin, OUTPUT);
  pinMode(rightBackwardPin, OUTPUT);

  digitalWrite(rightEnablePin, LOW);

  pinMode(leftEnablePin, OUTPUT);
  pinMode(leftForwardPin, OUTPUT);
  pinMode(leftBackwardPin, OUTPUT);

  digitalWrite(leftEnablePin, LOW);
}

void setupWiFi() {
  const char AP_NameChar[] = "hello";
  const char WIFI_PASSWORD[] = "iotfmi12345";

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_NameChar, WIFI_PASSWORD);
}

// rightSpeed, leftSpeed are in the range [0, 1023]
// rightDirection, leftDirection are in the range {FORWARDS, STOP, BACKWARDS}
void move(int rightDirection, int rightSpeed, int leftDirection, int leftSpeed) {

  int rightForwardPinSpeed = 0;
  int rightBackwardPinSpeed = 0;

  int leftForwardPinSpeed = 0;
  int leftBackwardPinSpeed = 0;

  if (rightDirection == FORWARDS) {
    rightForwardPinSpeed = 1023;
    rightBackwardPinSpeed = 0;
  } else if (rightDirection == BACKWARDS) {
    rightForwardPinSpeed = 0;
    rightBackwardPinSpeed = 1023;
  } else {
    rightForwardPinSpeed = 0;
    rightBackwardPinSpeed = 0;
  }


  if (leftDirection == FORWARDS) {
    leftForwardPinSpeed = 1023;
    leftBackwardPinSpeed = 0;
  } else if (leftDirection == BACKWARDS) {
    leftForwardPinSpeed = 0;
    leftBackwardPinSpeed = 1023;
  } else {
    leftForwardPinSpeed = 0;
    leftBackwardPinSpeed = 0;
  }

  analogWrite(rightForwardPin, rightForwardPinSpeed);
  analogWrite(rightBackwardPin, rightBackwardPinSpeed);
  analogWrite(rightEnablePin, rightSpeed);

  analogWrite(leftForwardPin, leftForwardPinSpeed);
  analogWrite(leftBackwardPin, leftBackwardPinSpeed);
  analogWrite(leftEnablePin, leftSpeed);

  Serial.print(" rightForward = ");
  Serial.print(rightForwardPinSpeed);

  Serial.print(" rightBackward = ");
  Serial.println(rightBackwardPinSpeed);

  Serial.print(" leftForward = ");
  Serial.print(leftForwardPinSpeed);

  Serial.print(" leftBackward = ");
  Serial.println(leftBackwardPinSpeed);


}

// if no request, return 0
// else, return 1
int getSpeedAndDirection() {
  const String httpFileNotFound = "HTTP/1.1 404 Not Found\r\n\r\n";
  const String httpOK = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    delay(100);
    Serial.print(".");
    return 0;
  }

  Serial.println();
  Serial.println("Client is connected");

  String request = client.readStringUntil('\r');

  Serial.println(request);

  String answer ;

  if (request.startsWith("GET / ")) {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
      Serial.println("File not open");
      client.print(httpFileNotFound);
      
    } else {
      Serial.println("file is OK");
      client.print(httpOK);
      
      int symbol;
      while ((symbol = file.read()) != -1) {
        Serial.print((char)symbol);
        client.print((char)symbol);
      }
      client.println();
      file.close();
    }
  } else {
    int speedBeginIndex = request.indexOf(speedString) + speedStringLength;
    int speedEndIndex = request.indexOf("/", speedBeginIndex);

    String tmp = request.substring(speedBeginIndex, speedEndIndex);
    speed = tmp.toInt();

    int directionBeginIndex = request.indexOf(directionString) + directionStringLength;
    int directionEndIndex = request.indexOf("/", directionBeginIndex);

    tmp = request.substring(directionBeginIndex, directionEndIndex);
    direction = tmp.toInt();

    Serial.print("Speed = ");
    Serial.print(speed);

    Serial.print(", direction = ");
    Serial.println(direction);

    client.print(httpFileNotFound);
  }

  return 1;
}

void calculateSpeedAndDirection() {

  int rightDirection;
  int leftDirection;

  int rightSpeed;
  int leftSpeed;

  if (speed > 0) {
    rightDirection = FORWARDS;
    leftDirection = FORWARDS;
  } else if (speed < 0) {
    rightDirection = BACKWARDS;
    leftDirection = BACKWARDS;
  } else {
    rightDirection = STOP;
    leftDirection = STOP;
  }

  if (speed < 0) {
    speed = -speed;
  }

  rightSpeed = speed * 1023 / 100;
  leftSpeed = speed * 1023 / 100;

  if (direction < 0) {
    //turn left
    //left motor must lower its speed
    direction = -direction;

    leftSpeed = leftSpeed * (100 - direction) / 100;
  } else if (direction > 0) {
    //turn right
    //right motor must lower its speed

    rightSpeed = rightSpeed * (100 - direction) / 100;
  }

  move(rightDirection, rightSpeed, leftDirection, leftSpeed);
}

void loop() {
  int result = getSpeedAndDirection();

  if (result == 0) {
    return;
  }

  // make sure that speed and direction are in the range [-100, 100]
  if (speed > 100) {
    speed = 100;
  } else if (speed < -100) {
    speed = -100;
  }

  if (direction > 100) {
    direction = 100;
  } else if (direction < -100) {
    direction = -100;
  }

  calculateSpeedAndDirection();
}
