#include "DHT.h"

/* Měření okolí */
#define DHTPIN 8
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE); 
/****************/

/* Větrák */
const int FANPIN = 9;

float fanTemp = 25.0;
float fanHum = 50.0;

float fanTempHysteresis = 0.5;
float fanHumHysteresis = 5.0;

boolean fanUsable = true;
/**********/

/* Květináč */
const int POTSOILPIN[] = {A0};
const int POTWATERPIN[] = {A1};
const int POTCOUNT = 1;   // Nutné změnit na odpovídající počet květináčů v předchozích polích
/************/


unsigned long previousMillis = 0;
String incoming = "";


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  potSetup();

  DHTsetup();

  fanSetup();
}

void loop() {
  unsigned long currentMillis = millis();

  while(Serial.available()) {
      char character = Serial.read();
      
      if (character == '^') {
        incoming = "";
      }

      if ((character == '^' || incoming.indexOf("^") == 0) && incoming.indexOf("$") == -1) {
        incoming += character;
      }
  }
  
  fanLoop();

  if (previousMillis == 0 || (currentMillis - previousMillis) >= 2000) {
//    Serial.print(incoming);

    /* Měření okolí + Květináč => posílání zpráv na sériový monitor */
    String dhtData = "^" + DHTloop() + "|" + potLoop() + "$";

    Serial.println(dhtData);
    /****************/

    previousMillis = currentMillis;
  }
}


/*****************************************************/
// Měření okolí

void DHTsetup() {
  dht.begin();
}

String DHTloop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return "";
  } 
  
  return "{\"humidity\":" + (String)h + ",\"temperature\":" + (String)t + "}";
}


/******************************************************/
// Květináč

void potSetup() {
  for (int pot : POTSOILPIN) {
    pinMode(pot, INPUT);
  }  
  for (int pot : POTWATERPIN) {
    pinMode(pot, INPUT);
  }  
}

String potLoop() {
  String values = "[";

  int resval = 0;
  int sensorSoil = 0;

  
  for (int i = 0; i < POTCOUNT; i++) {
    //mereni hladiny
    resval = analogRead(POTWATERPIN[i]);

    //mereni vlhkosti pudy, 595 je sucho, 239 je mokro
    sensorSoil = analogRead(POTSOILPIN[i]);

    for (int y = 0; y < 5; y++) {
      resval = (resval + analogRead(POTWATERPIN[i])) / 2;
      sensorSoil = (sensorSoil + analogRead(POTSOILPIN[i])) / 2;
    }

    values += String("{\"sh\":") + map(sensorSoil, 0,1023,100,0) + String(",\"wl\":") + map(resval, 0,650,0,100) + "}";

    if (i < POTCOUNT - 1) {
      values += ",";  
    }
  }
  
  return values + "]";
}

/******************************************************/
// Větrák

void fanSetup() {
  pinMode(FANPIN, OUTPUT);
}

void fanLoop() {
  // Reset incoming pokud je celá zpráva
  if (incoming.indexOf("^") == 0 && incoming.indexOf("$") == incoming.length() - 1 && incoming.indexOf("$") > 0) {
    
    // Nastavení hodnot větráku ze zpráv sériového portu
    String topic = incoming.substring(1, 8);

    if (topic == "fan/use") {
      String message = incoming.substring(9, incoming.length() - 1);
      fanUsable = message == "true";
      Serial.println(fanUsable);
    }
    else if (topic == "fan/tem") {
      String message = incoming.substring(9, incoming.length() - 1);
      fanTemp = message.toFloat();
      Serial.println(fanTemp);
    }
    else if (topic == "fan/hum") {
      String message = incoming.substring(9, incoming.length() - 1);
      fanHum = message.toFloat();
      Serial.println(fanHum);
    }
    else if (topic == "fan/thy") {
      String message = incoming.substring(9, incoming.length() - 1);
      fanTempHysteresis = message.toFloat();
      Serial.println(fanTempHysteresis);
    }
    else if (topic == "fan/hhy") {
      String message = incoming.substring(9, incoming.length() - 1);
      fanHumHysteresis = message.toFloat();
      Serial.println(fanHumHysteresis);
    }
    
    incoming = "";
  }
  
  // Zapnutí / vypnutí větráku
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 

  if (fanUsable == true || fanUsable == 1) {
      if (h > (fanHum + fanHumHysteresis) || t > (fanTemp + fanTempHysteresis)) {
      //  digitalWrite(FANPIN, HIGH);
        analogWrite(FANPIN, 900);
      }
      else if (h < (fanHum - fanHumHysteresis) || t < (fanTemp - fanTempHysteresis)) {
      //  digitalWrite(FANPIN, LOW);
        analogWrite(FANPIN, 10);
      }
  }
  else {
    //digitalWrite(FANPIN, LOW);
    analogWrite(FANPIN, 10);
  }
}

