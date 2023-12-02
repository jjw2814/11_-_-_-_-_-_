#include <FS.h>

#include <PubSubClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h> 
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Pin match
#define D0 16
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define LED_PIN D0
#define GAS_PIN A0
#define DHT_PIN D3
#define IR_O_PIN D5
// #define IR_I_PIN D6

#define DHTTYPE DHT22

//OLED
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//sensors
unsigned long previousMillis_DHT = 0;
const long interval_DHT = 3000;
unsigned long previousMillis_IR = 0;
const long interval_IR = 1000;
float temperature = 0, humidity = 0;
int people_count = 0;
int iSensorState;
int oSensorState;
int gas_data = 0;
//json
DHTesp dht;
StaticJsonDocument<256> dhtJson;
JsonObject data = dhtJson.createNestedObject("data");

StaticJsonDocument<256> gasJson;
JsonObject data2 = gasJson.createNestedObject("data");

StaticJsonDocument<256> peopleJson;
JsonObject data3 = peopleJson.createNestedObject("data");

// Publish
const char *pub_dht = "iot/21900059/dht22";
const char *pub_gas = "iot/21900059/gas";
const char *pub_people = "iot/21900059/people";


// Subscribe
const char *sub_topic = "iot/21900059/bus_info";

struct WifiSettings {
  char apName[20] = "espAP";
  char apPassword[20] = "esp12345";
};

// Here you can pre-set the settings for the MQTT connection. The settings can later be changed via Wifi Manager.
struct MqttSettings {
  char clientId[20] = "ryun MCU";
  char hostname[40] = "sweetdream.iptime.org";
  char port[6] = "1883";
  char user[20] = "iot";
  char password[20] = "csee1414";
  char wm_client_id_identifier[15] = "client_id";
  char wm_mqtt_hostname_identifier[14] = "mqtt_hostname";
  char wm_mqtt_port_identifier[10] = "mqtt_port";
  char wm_mqtt_user_identifier[10] = "mqtt_user";
  char wm_mqtt_password_identifier[14] = "mqtt_password";
};

// save config to file
bool shouldSaveConfig = false;

WiFiClient espClient;
WifiSettings wifiSettings;
PubSubClient client(espClient);
MqttSettings mqttSettings;

void readSettingsFromConfig() {
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        // Use arduinojson.org/v6/assistant to compute the capacity.
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, configFile);
        if (error) {
          Serial.println(F("Failed to read file, using default configuration"));
        } else {
          Serial.println("\nparsed json");

          strcpy(mqttSettings.clientId, doc[mqttSettings.wm_client_id_identifier]);
          strcpy(mqttSettings.hostname, doc[mqttSettings.wm_mqtt_hostname_identifier]);
          strcpy(mqttSettings.port, doc[mqttSettings.wm_mqtt_port_identifier]);
          strcpy(mqttSettings.user, doc[mqttSettings.wm_mqtt_user_identifier]);
          strcpy(mqttSettings.password, doc[mqttSettings.wm_mqtt_password_identifier]);
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void initializeWifiManager() {
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_client_id("client_id", "mqtt client id", mqttSettings.clientId, 40);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqttSettings.hostname, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqttSettings.port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqttSettings.user, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqttSettings.password, 20);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_client_id);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);

  if (!wifiManager.autoConnect(wifiSettings.apName, wifiSettings.apPassword)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  //read updated parameters
  strcpy(mqttSettings.clientId, custom_client_id.getValue());
  strcpy(mqttSettings.hostname, custom_mqtt_server.getValue());
  strcpy(mqttSettings.port, custom_mqtt_port.getValue());
  strcpy(mqttSettings.user, custom_mqtt_user.getValue());
  strcpy(mqttSettings.password, custom_mqtt_pass.getValue());
}

void saveConfig() {
  Serial.println("saving config");
    StaticJsonDocument<1024> doc;
    doc[mqttSettings.wm_client_id_identifier] = mqttSettings.clientId;
    doc[mqttSettings.wm_mqtt_hostname_identifier] = mqttSettings.hostname;
    doc[mqttSettings.wm_mqtt_port_identifier] = mqttSettings.port;
    doc[mqttSettings.wm_mqtt_user_identifier] = mqttSettings.user;
    doc[mqttSettings.wm_mqtt_password_identifier] = mqttSettings.password;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    configFile.close();
}

void initializeMqttClient() {
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  client.setServer(mqttSettings.hostname, atoi(mqttSettings.port));
  client.subscribe(sub_topic);
}

bool tryConnectToMqttServer() {
  if(strlen(mqttSettings.user) == 0) {
    return client.connect(mqttSettings.clientId);
  } else {
    return client.connect(mqttSettings.clientId, mqttSettings.user, mqttSettings.password);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (tryConnectToMqttServer()) {
      Serial.println("connected");
      client.subscribe(sub_topic);
    } else {
      Serial.print(mqttSettings.hostname);
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handleMqttState() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // if(strcmp(topic,"iot/21900059") != 0){
  //   return;
  // }
  Serial.println((char*)payload);
  const size_t bufferSize = JSON_OBJECT_SIZE(2);
  DynamicJsonDocument jsonBuffer(bufferSize);

  // JSON 파싱
  DeserializationError error = deserializeJson(jsonBuffer, payload);

  if (!error) {
    // 파싱된 데이터 가져오기
    const char* value302 = jsonBuffer["302"];  
    const char* valueYangdeok = jsonBuffer["yangdeok"];  

    // 결과 출력
    Serial.print("302: ");
    Serial.println(value302);
    Serial.print("yangdeok: ");
    Serial.println(valueYangdeok);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("302: "));
    display.print(value302);
    // display.print(char(248)) ;
    display.println(F(" min"));
    display.print(F("yangdeok: "));
    display.print(valueYangdeok);
    // display.print(char(248)) ;
    display.println(F(" min"));
    display.display();
  } else {
    Serial.println("JSON parsing failed!");
  }
}

void setup() {
  Serial.begin(115200);
  //Pin setup
  pinMode(GAS_PIN, INPUT);
  pinMode(IR_O_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  //dht setup
  dht.setup(DHT_PIN, DHTesp::DHTTYPE) ;

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  //indicate sensor type in json
  dhtJson["sensor"] = "dht22" ;
  gasJson["sensor"] = "gas" ;
  peopleJson["sensor"] = "people" ;

  //wifi and mqtt
  readSettingsFromConfig();
  initializeWifiManager();

  if (shouldSaveConfig) {
    saveConfig();
  }

  initializeMqttClient();
  client.setCallback(callback);
}

void loop() {
  handleMqttState();
  //get current time
  unsigned long currentMillis = millis();

  //store json data as String
  String dhtjsonData = "" ;
  String peoplejsonData = "" ;
  String gasjsonData = "" ;

  char buf[100];
  
  if (currentMillis - previousMillis_IR >= interval_IR) {
    previousMillis_IR = currentMillis;

    oSensorState = digitalRead(IR_O_PIN);
    sprintf(buf,"%d",oSensorState);
    client.publish(pub_people, buf) ;
    Serial.println(buf);
  }
  //Get DHT information every interval_DHT(milisec)
  //And publish jsons, dht data and cds data
  if (currentMillis - previousMillis_DHT >= interval_DHT) {
    previousMillis_DHT = currentMillis;
    gas_data = analogRead(GAS_PIN);

    humidity = dht.getHumidity();
    temperature = dht.getTemperature();

    int humidity_temp = humidity * 100 ;
    int temperature_temp = temperature * 100 ;
    data["humidity"] = humidity_temp / 100 ;
    data["temperature"] = temperature_temp / 100 ;
    serializeJsonPretty(dhtJson, dhtjsonData) ;
    dhtjsonData.toCharArray(buf, 100);
    client.publish(pub_dht, (uint8_t *)buf, dhtjsonData.length(), true) ;

    data2["concentration"] = gas_data;
    serializeJsonPretty(gasJson, gasjsonData) ;
    gasjsonData.toCharArray(buf, 100);
    sprintf(buf,"%d",gas_data);
    client.publish(pub_gas, buf) ;
  }
}
