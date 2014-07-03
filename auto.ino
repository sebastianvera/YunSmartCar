/*
 Smart Yun Car

 This is the API for controlling the TYCON RC AIR REBOUND car.

 Possible commands created in this sketch:

 * "/arduino/forward/fast"        -> The car moves forward, normal speed.
 * "/arduino/forward/slow"        -> The car moves forward, parking mode.
 * "/arduino/backward/fast"       -> The car moves backward, normal speed.
 * "/arduino/backward/slow"       -> The car moves backward, parking mode
 * "/arduino/left/fast"           -> The car goes left, normal speed.
 * "/arduino/left/slow"           -> The car goes left, parking mode
 * "/arduino/right/fast"          -> The car goes right, normal speed.
 * "/arduino/right/slow"          -> The car goes right, parking mode
 */

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <LiquidCrystal.h>


//Liquid Crystal Settings
const int RS = 4;
const int E = 8;
const int D4 = 3;
const int D5 = 12;
const int D6 = 11;
const int D7 = 13;
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

// Global variables
YunServer server;
boolean isGoingForward = false;
int status = 0;
const int RF = 10;
const int RB = 9;
const int LF = 6;
const int LB = 5;
const int fast = 255; // 255
const int slow = 150;
boolean firstTime = true;
int timeout = 0;
boolean on = false;
int count = 0;
const int frontPing = A4;
const int backPing = A5;
const int buzzerPin = 2;
const int frontThreshold = 100;
const int backThreshold = 100;
float frontDiff = 25;
float backDiff = 4;
float buzzerFrequencyInSeconds = 1;
String ip;

void setup() {
  pinMode(RF, OUTPUT);
  pinMode(RB, OUTPUT);
  pinMode(LF, OUTPUT);
  pinMode(LB, OUTPUT);
  
  
  // Bridge startup
  Bridge.begin();
  ip = getIP();
  
  lcd.begin(16, 2);
  lcd.print(ip);
  
  Serial.begin(9600);
  Serial.println(ip);
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
  Serial.println("Server running");
    // Print IP
  makeNoise();
  makeNoise();
  delay(3000);
  Serial.println(ip);
}

void loop() {
  if (timeout > 3) // 0.150 seconds
  {
    halt();
    firstTime = true;
  }

  if (timeout > 100) {
    timeout = 11;
  }
  processRequest();
  if (isGoingForward)
    measureFrontDistance();
  else
    measureBackDistance();
  delay(50); // Poll every 50ms
}

// ******************************************
// *                Functions               *
// ******************************************

void processRequest() {
  // Get clients coming from server
  YunClient client = server.accept();
  // There is a new client?
  if (client) {
    Serial.println("Incoming request.");
    // Process request
    process(client);
    // Close connection and free resources.
    client.stop();
  } else {
    timeout += 1;
  }
}

void process(YunClient client) {
  Serial.print("Before request - Status: "); // Debugging
  Serial.println(status);// Debugging
  timeout = 0;
  // read the command from URL
  String command = client.readStringUntil('/');
  String mode = client.readStringUntil('\r');

  halt();

  // process the request
  Serial.println(command);
  Serial.println(isGoingForward);
  if (command == "forward") {
    forward(mode);
  }
  if (command == "backward") {
    backward(mode);
  }
  if (command == "left") {
    left(mode);
  }
  if (command == "right") {
    right(mode);
  }
  client.print(isGoingForward);
  Serial.print("After request - Status: "); // Debugging
  Serial.println(status);// Debugging
}

void forward(String mode) {
  if (firstTime) {
    isGoingForward = true;
    firstTime = false;
  }

  status = 1;
  if (mode == "fast") {
    analogWrite(RF, fast-30);
    analogWrite(LF, fast-30);
  } else {
    analogWrite(RF, slow-30);
    analogWrite(LF, slow-30);
  }
}

void backward(String mode) {
  if (firstTime) {
    isGoingForward = false;
    firstTime = false;
  }

  if (isGoingForward) {
    halt();
    isGoingForward = false;
    status = 0;
    Serial.println("Frenar : Iba hacia adelante");
    return;
  }
  status = -1;
  if (mode == "fast") {
    analogWrite(RB, fast);
    analogWrite(LB, fast);
  } else {
    analogWrite(RB, slow);
    analogWrite(LB, slow);
  }
}

void left(String mode) {
  if (mode == "fast") {
    if (isGoingForward)
      digitalWrite(RF, fast);
    else
      digitalWrite(RB, fast);
  } else {
    if (isGoingForward)
      digitalWrite(RF, slow);
    else
      digitalWrite(RB, slow+30);
  }
}

void right(String mode) {
  if (mode == "fast") {
    if (isGoingForward)
      digitalWrite(LF, fast);
    else
      digitalWrite(LB, fast);
  } else {
    if (isGoingForward){
      digitalWrite(LF, slow);
    }
    else {
      digitalWrite(LB, slow + 30);
    }
  }
}

void halt() {
  digitalWrite(LF,LOW);
  digitalWrite(LB,LOW);
  digitalWrite(RF,LOW);
  digitalWrite(RB,LOW);
}

String getIP() {
  Process a;
  a.runShellCommand("ifconfig | grep inet | grep Bcast | awk '{print $2}' | awk -F ':' '{print $2}'");
  return(a.readStringUntil('\r'));
}

int frequency()
{
  if (buzzerFrequencyInSeconds <= 0)
    return 0;
  return buzzerFrequencyInSeconds*1000/50;
}

long getDistance(int pingPin) {
  
  long duration, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);
  // convert the time into a distance
  return microsecondsToCentimeters(duration);
}

long microsecondsToCentimeters(long microseconds)
{
  return microseconds / 29 / 2;
}

void measureFrontDistance() {
  long frontPingDistance = getDistance(frontPing);
  if (frontPingDistance - frontDiff <= frontThreshold) {
    // Start sensor
    if (!on) 
      count = 0;
    on = true;
    buzzerFrequencyInSeconds = ((float)frontPingDistance - frontDiff)/(float)frontThreshold;
  }else{
    on = false;
    buzzerFrequencyInSeconds = 1;
  }
  
  if (on && count >= frequency()) {
    Serial.println("PI!!!! FRONT");
    makeNoise();
    count = 0;
  } else {
    count++;
  }
}

void measureBackDistance() {
  long backPingDistance = getDistance(backPing);
  if (backPingDistance - backDiff <= backThreshold) {
    // Start sensor
    if (!on) 
      count = 0;
    on = true;
    buzzerFrequencyInSeconds = ((float)backPingDistance - backDiff)/(float)backThreshold;
  }else{
    on = false;
    buzzerFrequencyInSeconds = 1;
  }
  
  if (on && count >= frequency()) {
    Serial.println("PI!!!! BACK");
    makeNoise();
    count = 0;
  } else {
    count++;
  }
}

void makeNoise() {
//  tone(buzzerPin, 131, 100);
}
