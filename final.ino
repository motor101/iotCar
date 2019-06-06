#include <ESP8266WiFi.h>

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
  Serial.begin(115200);
  Serial.print("Waiting for client ");
  
  setupWiFi();
  server.begin();
  
  pinMode(rightEnablePin,OUTPUT);
  pinMode(rightForwardPin,OUTPUT);
  pinMode(rightBackwardPin,OUTPUT);

  digitalWrite(rightEnablePin, LOW);

  pinMode(leftEnablePin,OUTPUT);
  pinMode(leftForwardPin,OUTPUT);
  pinMode(leftBackwardPin,OUTPUT);

  digitalWrite(leftEnablePin, LOW);
}

void setupWiFi(){
  const char AP_NameChar[]="hello";
  const char WIFI_PASSWORD[] = "iotfmi12345";

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_NameChar, WIFI_PASSWORD);
}

// rightSpeed, leftSpeed are in the range [0, 1023]
// rightDirection, leftDirection are in the range {FORWARDS, STOP, BACKWARDS}
void move(int rightDirection, int rightSpeed, int leftDirection, int leftSpeed){

  int rightForwardPinSpeed = 0;
  int rightBackwardPinSpeed = 0;
  int leftForwardPinSpeed = 0;
  int leftBackwardPinSpeed = 0;
  
  if(rightDirection == FORWARDS){
    rightForwardPinSpeed = rightSpeed;
    rightBackwardPinSpeed = 0;
  }else if (rightDirection == BACKWARDS){
    rightForwardPinSpeed = 0;
    rightBackwardPinSpeed = rightSpeed;
  } else {
    rightForwardPinSpeed = 0;
    rightBackwardPinSpeed = 0;  
  }

  
  if(leftDirection == FORWARDS){
    leftForwardPinSpeed = leftSpeed;
    leftBackwardPinSpeed = 0;
  }else if (leftDirection == BACKWARDS){
    leftForwardPinSpeed = 0;
    leftBackwardPinSpeed = leftSpeed;
  } else {
    leftForwardPinSpeed = 0;
    leftBackwardPinSpeed = 0;  
  }

  analogWrite(rightForwardPin, rightForwardPinSpeed);
  analogWrite(rightBackwardPin, rightBackwardPinSpeed);
  digitalWrite(rightEnablePin, HIGH);
  
  analogWrite(leftForwardPin, leftForwardPinSpeed);
  analogWrite(leftBackwardPin, leftBackwardPinSpeed);
  digitalWrite(leftEnablePin, HIGH);

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
int getSpeedAndDirection(){
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
  
  if(request.startsWith("GET / ")){
    answer = "HTTP/1.1 200 OK\r\n"             \
             "Content-Type: text/html\r\n\r\n" \
             "<!DOCTYPE html><head>    <meta name='viewport' content='width=device-width, initial-scale=1.0'>    <meta charset='utf8' />    <style>        * {            box-sizing: border-box;        }        nav {            float: left;            width: 50%;            padding: 20px;            list-style-type: none;            padding: 0;        }        article {            text-align: right;            float: left;            padding: 20px;            width: 50%;        }        header {            text-align: center;            background-color: #666;            padding: 30px;            text-align: center;            font-size: 35px;        }        .slider {            -webkit-appearance: none;            width: 100%;            height: 25px;            background: #d3d3d3;            outline: none;            opacity: 0.7;            -webkit-transition: .2s;            transition: opacity .2s;        }        .slider:hover {            opacity: 1;        }        .slider::-webkit-slider-thumb {            -webkit-appearance: none;            appearance: none;            width: 25px;            height: 25px;            background: #4CAF50;            cursor: pointer;        }        .slider::-moz-range-thumb {            width: 25px;            height: 25px;            background: #4CAF50;            cursor: pointer;        }    </style></head><body>    <script>        function httpGet(theUrl) {            var xmlHttp = new XMLHttpRequest();            xmlHttp.open('GET', theUrl, true);            xmlHttp.send(null);        }        function udpateCar() {            speed = document.getElementById('speed').value;            document.getElementById('speedOutput').innerText = speed;            direction = document.getElementById('direction').value;            document.getElementById('directionOutput').innerText = direction;            httpGet(window.location + 'speed=' + speed + '/direction=' + direction + '/');        }        function stopCar() {            document.getElementById('speed').value = 0;            document.getElementById('direction').value = 0;            udpateCar();        }    </script>    <header>        <button style='font-size: 100%' onclick='stopCar()'>STOP</button>    </header>    <section>        <nav><input class='slider' id='direction' oninput='udpateCar()' type='range' min='-100' max='100' value='0'>            <h1>direction = <span id='directionOutput'>0</span></h1>        </nav>        <article><input class='slider' id='speed' oninput='udpateCar()' type='range' min='-100' max='100' value='0'>            <h1>speed = <span id='speedOutput'>0</span></h1>        </article>    </section></body></html>";
  } else {
    int speedBeginIndex = request.indexOf(speedString)+speedStringLength;
    int speedEndIndex = request.indexOf("/", speedBeginIndex);
    
    String tmp = request.substring(speedBeginIndex, speedEndIndex);
    speed = tmp.toInt();

    int directionBeginIndex = request.indexOf(directionString)+directionStringLength;
    int directionEndIndex = request.indexOf("/", directionBeginIndex);
    
    tmp = request.substring(directionBeginIndex, directionEndIndex);
    direction = tmp.toInt();


    Serial.print("Speed = ");
    Serial.print(speed);
    
    Serial.print(", direction = ");
    Serial.println(direction);
    
    answer = "HTTP/1.1 404 Not Found\r\n\r\n";  
  }
  
  client.print(answer);

  return 1;
}

void calculateSpeedAndDirection(){

  int rightDirection;
  int leftDirection;
  
  int rightSpeed;
  int leftSpeed;  

  if(speed > 0){
    rightDirection = FORWARDS;
    leftDirection = FORWARDS;
  } else if(speed < 0){
    rightDirection = BACKWARDS;
    leftDirection = BACKWARDS;
  } else{  
    rightDirection = STOP;
    leftDirection = STOP;
  }

  if(speed < 0){
    speed = -speed;
  }

  rightSpeed = speed*1023/100;
  leftSpeed = speed*1023/100;

  if(direction < 0){
    //turn left
    //left motor must lower its speed
    direction = -direction;

    leftSpeed = leftSpeed*(100-direction)/100;
  }else if(direction > 0){
    //turn right
    //right motor must lower its speed

    rightSpeed = rightSpeed*(100-direction)/100;
  } 

  move(rightDirection, rightSpeed, leftDirection, leftSpeed);
}

void loop() {  
  int result = getSpeedAndDirection();

  if(result == 0){
    return;
  }
  
  // make sure that speed and direction are in the range [-100, 100]
  if(speed > 100){
    speed = 100;
  }else if (speed < -100){
    speed = -100;
  }

  if(direction > 100){
    direction = 100;
  }else if (direction < -100){
    direction = -100;
  }

  calculateSpeedAndDirection();
}
