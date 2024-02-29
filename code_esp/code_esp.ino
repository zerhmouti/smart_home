#include <AccelStepper.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "WIFI NAME"
#define WIFI_PASSWORD "WIFI PASSWOR"
#define API_KEY "put your api key here"
#define DATABASE_URL "load-project-21787-default-rtdb.firebaseio.com" 

const int stepsPerRevolution = 2048; 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

#define Relay1_kitchen D8
#define Relay2_garage D7
#define Relay3_fun D3
#define wifiLed D4

bool load1 = false;
bool load2 = false;
bool load3 = false;
int m1_position=0;

// ULN2003 Motor Driver Pins
#define IN1 5  //D1
#define IN2 4  //D2
#define IN3 14  //D5
#define IN4 12  //D6

AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

void setup() {
  pinMode(Relay1_kitchen, OUTPUT);
  pinMode(Relay2_garage, OUTPUT);
  pinMode(Relay3_fun, OUTPUT);
  pinMode(wifiLed, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stepper.setMaxSpeed(500);
  stepper.setAcceleration(100);
  stepper.setCurrentPosition(0);

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  digitalWrite(wifiLed, HIGH);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  if (Firebase.ready()) {
    readFirebaseData();
    // Uncomment below lines if you need to control relays
    digitalWrite(Relay1_kitchen, load1 ? LOW : HIGH);
    digitalWrite(Relay2_garage, load2 ? LOW : HIGH);
    digitalWrite(Relay3_fun, load3 ? LOW : HIGH);
  }

  delay(3000); // Wait for 3 seconds before next iteration
}

void readFirebaseData() {
  if (Firebase.RTDB.getString(&fbdo, "/testproject/L1")) {
    if (fbdo.dataType() == "string") {
      load1 = fbdo.stringData() == "1";
      Serial.println("load1: " + String(load1));
    }
  } else {
    Serial.println(fbdo.errorReason());
  }
  if (Firebase.RTDB.getString(&fbdo, "/testproject/L2")) {
    if (fbdo.dataType() == "string") {
      load2 = fbdo.stringData() == "1";
      Serial.println("Load2: " + String(load2));
    }
  } else {
    Serial.println(fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/testproject/L4")) {
    if (fbdo.dataType() == "string") {
      load3 = fbdo.stringData() == "1";
      Serial.println("Load3: " + String(load2));
    }
  } else {
    Serial.println(fbdo.errorReason());
  }

  if (Firebase.RTDB.getString(&fbdo, "/testproject/M1")) {
    int m1t= atoi(fbdo.stringData().c_str());
    if(m1t> m1_position){
      moveStepperToPosition(m1t-m1_position);
      m1_position=m1t-m1_position ;
    }else if(m1t<m1_position){
      moveStepperToPosition(-(m1_position-m1t));
      m1_position=m1t;
    }
    Serial.println("M1: " + String(m1_position));
    
  } else {
    Serial.println(fbdo.errorReason());
  }
}

void moveStepperToPosition(int targetPosition) {
  stepper.moveTo(targetPosition);
  
  // while (stepper.distanceToGo() != 0) {
  //   stepper.run();
  // }
  stepper.move(targetPosition);
  stepper.runToPosition();
  
  Serial.print("Current Position: ");
  Serial.println(stepper.currentPosition());
}
