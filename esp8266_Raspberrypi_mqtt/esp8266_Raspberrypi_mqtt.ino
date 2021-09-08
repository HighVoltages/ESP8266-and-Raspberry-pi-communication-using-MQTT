#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <DHT.h>
#include <PubSubClient.h>

#define DHTPIN 5 // Digital pin connected to the DHT sensor
#define LED D2
// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);

unsigned long previousMillis = 0;
unsigned long interval = 10000;

const char* mqttServer = "192.168.0.110";
const int mqttPort = 1883;

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
    Serial.println();
  
  String message = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message+=(char)payload[i];
  }
  Serial.println("-----------------------");
  if(String(topic)=="LED"){
    if(message=="LED ON"){
      digitalWrite(LED,HIGH);
      Serial.println("LED IS ON");
    }
    else{
      digitalWrite(LED,LOW);
    }
  }
    
}

WiFiClient espClient;
PubSubClient client(mqttServer, mqttPort, callback, espClient);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
// wifiManager.resetSettings();

  //set custom ip for portal
  //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("AutoConnectAP");
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();


  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  dht.begin();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
    while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client" )) {
      client.subscribe("LED");
      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
}
}

void loop() {
    client.loop();
unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if(!isnan(h)&&!isnan(t)){
      Serial.println("Temperature : " +String(t));
      Serial.println("Humidity : " +String(h));
      String toSend = String(t) + "," +String(h);
      client.publish("data",toSend.c_str());
    }

  }
}
