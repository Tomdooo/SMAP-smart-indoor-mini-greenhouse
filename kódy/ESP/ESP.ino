#include <ESP8266WiFi.h>    //Je součástí balíčku knihoven pro desky ESP8266
#include <PubSubClient.h>   //Je potřeba samostatně nainstalovat

// WiFi
const char* ssid        = "LAPTOP-A2VH7U0H_3130";
const char* password    = "4W+534h9";

// Připojení k MQTT
const char* mqtt_server = "192.168.137.1";
const int   mqtt_port = 1883;
const char* mqttUser = "client";
const char* mqttPassword = "test";

// MQTT topicy k odebírání
const char* TOPIC1 = "fan/use";
const char* TOPIC2 = "fan/temp";
const char* TOPIC3 = "fan/hum";
const char* TOPIC4 = "fan/tempHys";
const char* TOPIC5 = "fan/humHys";

unsigned long lastPublish = 0;

const int msgLength = 38;
String incoming = "";


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);

  Serial.println("I am alive!");

  WiFi.mode(WIFI_STA);                //Mód Wifi STA = station
  mqttClient.setServer(mqtt_server, mqtt_port); //Připojení k brokeru
  mqttClient.setCallback(callback);        //Funkce pro subscribe topiců z brokeru 
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {     //Znovupřipojení k wifi pokud dojde k výpadku
    WiFiSetup();
  }
  if (!mqttClient.connected()) {          //Znovu připojení k brokeru pokud dojde ke ztrátě spojení
    mqttConnect();
    mqttClient.subscribe(TOPIC1);
    mqttClient.subscribe(TOPIC2);
    mqttClient.subscribe(TOPIC3);
    mqttClient.subscribe(TOPIC4);
    mqttClient.subscribe(TOPIC5);
  }
  mqttClient.loop();


  while(Serial.available()) {
    // read the incoming byte:
//    char message[msgLength];
//    Serial.readBytes(message, msgLength);
//    incoming = String(message);
//    Serial.println(incoming);

      char character = Serial.read();
      
      if (character == '^') {
          incoming = "";
      }

      incoming += character;
      
//      Serial.println(incoming);
  }


  //---- Sekce Publish ----//
  if (incoming.indexOf("^") == 0 && incoming.indexOf("$") == (incoming.length() - 1) && incoming.indexOf("$") != -1) {             //K zasílání dat bude docházet každé 2 sekundy
    String surroundings = "";
    String pots = "";

    
    if (incoming.indexOf("|") != -1) {
      surroundings = incoming.substring(1, incoming.indexOf("|"));
      pots = incoming.substring(incoming.indexOf("|") + 1, incoming.length() - 1);
    }
    else {
      surroundings = incoming.substring(1, incoming.length() - 1);
    }
    
    
    Serial.println(surroundings);
    Serial.println(pots);
    
//    Serial.println(incoming.substring(1, incoming.length() - 1));

    int str_len = surroundings.length() + 1; 
    char char_array[str_len];
    surroundings.toCharArray(char_array, str_len);
    
    mqttClient.publish("surroundings", char_array);     //Publikování hodnot okolí pod topicem surroundings
    

    if (pots.length() > 0) {
      int str_len = pots.length() + 1; 
      char char_array[str_len];
      pots.toCharArray(char_array, str_len);
      
      mqttClient.publish("pots", char_array);     //Publikování hodnot květináču pod topicem pots
    }
    
    incoming = "";
  }
}

/**********************************/
// WiFi setup

void WiFiSetup() {
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

/**********************************/
// MQTT Connect

void mqttConnect() {
  while (!mqttClient.connected()) {         //dokud není navázáno spojení s brokerem
    Serial.print("Připojování k MQTT brokeru...");
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX); //klientské ID (náhodně generované číslo)
    if (mqttClient.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      Serial.println("Úspěšně připojeno k brokeru");
    } else {
      Serial.print("připojení selhalo, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Zkuste znovu za 5s");
      delay(5000);
    }
  }
}

/****************************************/
// MQTT callback

void callback(String topic, byte* message, unsigned int length){
  //čtení prijaté zprávy a topicu
  Serial.print("Zpráva přijata na topic: ");
  Serial.print(topic);
  Serial.print(". Zpráva: ");
  String messageTemp;            //dočasná proměnná pro zprávu

  for(int i=0; i<length; i++){       //Ke čtení zprávy dochází po znakách
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();  


  if(topic=="fan/use"){
    String prnt = "^fan/use=" + messageTemp + "$";
    Serial.println(prnt);
  }
  
  else if(topic=="fan/temp"){
    String prnt = "^fan/tem=" + messageTemp + "$";
    Serial.println(prnt);
  }
  
  else if(topic=="fan/hum"){
    String prnt = "^fan/hum=" + messageTemp + "$";
    Serial.println(prnt);
  }

  else if(topic=="fan/tempHys"){
    String prnt = "^fan/thy=" + messageTemp + "$";
    Serial.println(prnt);
  }

  else if(topic=="fan/humHys"){
    String prnt = "^fan/hhy=" + messageTemp + "$";
    Serial.println(prnt);
  }
}

