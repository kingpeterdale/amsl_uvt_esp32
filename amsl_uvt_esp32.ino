#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <ESP32Servo.h> // Use v 3.0.3 of library

#include "WiFi.h"
#include "WebServer.h"
#include <ArduinoJson.h>

#include "WebIF.h"
#include "PID.h"

// System constants
const unsigned long DELAY_MS      = 50;
const unsigned long UPDATE_MS     = 100;
const unsigned long MAX_CONTROL   = 400;
const unsigned long MAX_ELEV_ANG  = 30;
const unsigned long MAX_RUD_ANG   = 60;
const float         RUD_TO_SERVO  = MAX_CONTROL / MAX_RUD_ANG;
const float         ELEV_TO_SERVO = MAX_CONTROL / MAX_ELEV_ANG;
const char * ssid = "AMSLBOT";
const char * password = "AMCAMSL7248";

// Pin numbers (USING ESP32 GPIO Numbering)
// D2 = 5
// D3 = 6
// D4 = 7
// D5 = 8
// D6 = 9
const unsigned int  THRUST        = 5;
const unsigned int  RUDDER        = 6;
const unsigned int  ELEVATOR      = 7;

Servo thruster;
Servo rudder;
Servo elevator;

Adafruit_BNO055 bno = Adafruit_BNO055(-1, 0x28, &Wire);

WebServer server(80);

unsigned long last_update = millis();

// System State
uint8_t sys, gyro, accel, mag ;
float hdg, pitch;
int rud, elev, thrust;

// User Settings
bool test_running = false;
unsigned long test_start = 0;

//Heading
bool hdg_en = true;
float hdg_sp = 0.0;
float hdg_kp = 0.0;
float hdg_ki = 0.0;
float hdg_kd = 0.0;

// Pitch
bool pitch_en = true;
float pitch_sp = 0.0;
float pitch_kp = 0.0;
float pitch_ki = 0.0;
float pitch_kd = 0.0;

//Thruster
int thruster_sp = 1600;
unsigned long thruster_start = 0;
unsigned long thruster_stop = 0;

//Rudder
int rudder_sp = 0;
int rudder_next = 0;
unsigned long rudder_change = 0;

//Elevator
int elevator_sp = 0;

PID hdg_pid(hdg_kp, hdg_ki, hdg_kd, 100);
PID pitch_pid(pitch_kp, pitch_ki, pitch_kd,100);


void setup() {
  // Initialisation 
  Serial.begin(115200);
  WiFi.mode (WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to WiFi");
    delay(1000);
  }
  WiFi.setSleep(false);
  Serial.println(WiFi.localIP().toString());
  
  while(!bno.begin()) {
    // Do not proceed without IMU
    Serial.println("BNO055 not detected ...");
    delay(1000);
  }

  // Configure Web Interface
  server.on("/", handleRoot);
  server.on("/state", handleState);
  server.on("/test", HTTP_PUT,handleTest);
  server.begin();

  thruster.setPeriodHertz(50);
  thruster.attach(THRUST, 1500, 1700);
  thruster.write(1500);

  rudder.setPeriodHertz(50);
	rudder.attach(RUDDER, 1500 - MAX_CONTROL, 1500 + MAX_CONTROL);
  rudder.write(1500);
  
  elevator.setPeriodHertz(50);
	elevator.attach(ELEVATOR, 1500 - MAX_CONTROL, 1500 + MAX_CONTROL);
  elevator.write(1500);
  delay(1000);
}

void loop() {
  // Main program loop
  
  // Get values from IMU
  bno.getCalibration(&sys, &gyro, &accel, &mag);
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  
  // Peform any conversion or normalisation
  hdg = euler.x();
  pitch = euler.y();
  if (hdg > 180)
    hdg = hdg - 360;
  
  
  unsigned long elapsed = millis() - test_start;
  
  // If enabled, determine control output from PIDs
  if (pitch_en) {
    elev = int(1500 + 4 * pitch_pid.run(pitch_sp, pitch, DELAY_MS));
  } else {
    elev = 1500 + 4 * elevator_sp;
  }
  if (hdg_en) {
    rud  = int(1500 +  4 * hdg_pid.run(hdg_sp, hdg, DELAY_MS));
  } else {
    rud = 1500 + 4 * rudder_sp;
    if (test_running){
      if (elapsed > rudder_change) 
        rud = rudder_next;
    }
  }

  if (test_running) {
    if (elapsed > thruster_stop) {
      thrust = 1500;
      test_running = false;
    }
    else if (elapsed > thruster_start) 
      thrust = thruster_sp;
    else 
      thrust = 1500;;
  } else {
    thrust = 1500;
  }
  Serial.printf("%d %d %d %d %d\n", test_running, elapsed, thrust, rud, elev) ;
  
  rudder.write(rud);
  elevator.write(elev);
  thruster.write(thrust);
  
  //Serial.printf("Thrust: %d Rudder: %d Elevator: %d\n",1500, rud, elev);
  // Handle UI
  server.handleClient();
  server.client().flush();
  server.client().stop();

  // Delay to maintain update rate
  delay(DELAY_MS);
}

void handleRoot() {
  Serial.println("Root Request");
  String html = getHTML();
  server.send(200, "text/html", html);
}

void handleState() {
  char response[128];
  sprintf(response,"{\"hdg\": \"%+04.0f\", \"pitch\": \"%+06.1f\", \"cal\": \"%u\", \"rud\": \"%d\", \"elev\": \"%d\"}", hdg, pitch, sys, rud, elev);
  server.send(200, "application/json", response);
}

void handleTest() {
  char response[64];
  String json = server.arg("plain");
  Serial.println(json);
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, json);
  if (!err) {
    pitch_en = doc["pitch_en"];
    pitch_kp = atof(doc["pitch_kp"]);
    pitch_ki = atof(doc["pitch_ki"]);
    pitch_kd = atof(doc["pitch_kd"]);
    pitch_pid.updateGains(pitch_kp,pitch_ki,pitch_kd);
    Serial.println("Pitch parsed");

    hdg_en = doc["heading_en"];
    hdg_sp = hdg;
    hdg_kp = atof(doc["heading_kp"]);
    hdg_ki = atof(doc["heading_ki"]);
    hdg_kd = atof(doc["heading_kd"]);
    hdg_pid.updateGains(hdg_kp,hdg_ki,hdg_kd);
    Serial.println("Heading parsed");

    thruster_start = 1000 * atoi(doc["thrust_start"]);
    thruster_stop = 1000 * atoi(doc["thrust_stop"]);
    Serial.println("Thruster parsed");

    rudder_sp = atoi(doc["rudder_init"]);
    rudder_next = atoi(doc["rudder_change"]);
    rudder_change = 1000 * atoi(doc["rudder_change"]);
    Serial.println("Rudder parsed"); 

    elevator_sp = atoi(doc["rudder_init"]);

    sprintf(response, "{}");
    server.send(200, "application/json", response);
    test_start = millis();
    test_running = true;
  } else {
    server.send(400);
    test_running = false;
  }
  
}

