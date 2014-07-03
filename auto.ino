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

// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;
boolean isGoingForward = false;
int RF = 10;
int RB = 9;
int LF = 6;
int LB = 5;
int fast = 255; // 255
int slow = 150;
String lastRequest;
boolean firstTime;
int timeout;
bool on = false;
int count = 0;
const int fontPing = 7;
const int buzzerPin = 3;
int frontThreshold = 100;
float frontDiff = 25;
float buzzerFrequencyInSeconds = 1;

void setup() {
  firstTime = true;
  timeout = 0;
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
 
  pinMode(buzzerPin, OUTPUT);
  // Motor's pin
  pinMode(RF, OUTPUT);
  pinMode(RB, OUTPUT);
  pinMode(LF, OUTPUT);
  pinMode(LB, OUTPUT);

  // Bridge startup
  Bridge.begin();
  Serial.begin(9600);
  digitalWrite(13, HIGH);
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
  Serial.println("Server running");
}

void loop() {

  if (timeout > 10) // 0.5 seconds
  {
    halt();
    firstTime = true;
  }

  if (timeout > 100) {
    timeout = 11;
  }

  // Get clients coming from server
  YunClient client = server.accept();
  // There is a new client?
  if (client) {
    Serial.println("Incoming request.");
    digitalWrite(13, LOW);
    // Process request
    process(client);
    // Close connection and free resources.
    client.stop();
    digitalWrite(13, HIGH);
  } else {
    timeout += 1;
  }
  // Sensors logic with buzzer
  long frontPingDistance = getDistance(fontPing);
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
    tone(3, 131, 100);
    count = 0;
  } else {
    count++;
  }
  
  delay(50);
}

void process(YunClient client) {
  timeout = 0;
  // read the command from URL
  String command = client.readStringUntil('/');
  String mode = client.readStringUntil('\r');

  if (mode == lastRequest) {
    return;
  }
  
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
}

void forward(String mode) {
  if (firstTime) {
    isGoingForward = true;
    firstTime = false;
  }
  // Check if needs to halt
  if (!isGoingForward) {
    isGoingForward = true;
    halt();
    Serial.println("Frenar : Iba en reversa");
    return;
  }
  if (mode == "fast") {
    analogWrite(RF, fast);
    analogWrite(LF, fast);
  } else {
    analogWrite(RF, slow);
    analogWrite(LF, slow);
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
    Serial.println("Frenar : Iba hacia adelante");
    return;
  }

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
    // Go fast
  } else {
    // Go slow
  }
  digitalWrite(RF, HIGH);
//  digitalWrite(LB, HIGH);
}

void right(String mode) {
  if (mode == "fast") {
  } else {
  }

  digitalWrite(LF, HIGH);
//  digitalWrite(RB, HIGH);
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
