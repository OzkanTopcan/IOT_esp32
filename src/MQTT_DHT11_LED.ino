/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define WIFISSID "XXXXXX" // Put your WifiSSID here
#define PASSWORD "XXXXXX" // Put your wifi password here
#define TOKEN "XXXXXX" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "ETST_POW" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                    //it should be a random and unique ascii string and different from all other devices

/****************************************
 * Define Constants
 ****************************************/
#define DEVICE_LABEL "esp_mqtt" // Assig the device label
#define VARIABLE_LABEL "humidity" // Assign the variable label
#define VARIABLE_LABEL_2 "temperature" // Assign the variable label
#define VARIABLE_LABEL_3 "ioled" // Assign the variable label
#define VARIABLE_LABEL_4 "ledstate" // Assign the variable label
#define LED 2
float ledstate = 0;

const int DHTPIN = 4; // Pin where is connected the DHT11
const int DHTTYPE = DHT11; // Type of DHT

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
char topicSubscribe[100];

// Space to store values to send
char str_humidity[10];
char str_temperature[10];
char str_ledstate[10];

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);
DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");

  if ((char)payload[0] == '1') {
    digitalWrite(LED, HIGH);   
    ledstate=0;
  } else {
    digitalWrite(LED, LOW);  // Turn the LED off by making the voltage HIGH
    ledstate=1;
  }
 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  // Set pin mode
  pinMode(LED,OUTPUT);
  dht.begin();
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT 

  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LABEL_3);

  client.subscribe(topicSubscribe);
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  
}

void loop() {
  if (!client.connected()) {
    reconnect();
    client.subscribe(topicSubscribe);
  }
  
  /* Reading temperature and humidity */
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); 

   /* test si la lecture du sensor se fait pas */
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  
  /*******/
  /* 4 is mininum width, 2 is precision; float value is copied onto str_*/
  dtostrf(humidity, 4, 2, str_humidity);
  dtostrf(temperature, 4, 2, str_temperature);
  dtostrf(ledstate, 1, 0, str_ledstate);
  
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\": %s,", VARIABLE_LABEL, str_humidity); // Adds the variable label
  sprintf(payload, "%s\"%s\": %s,", payload, VARIABLE_LABEL_2, str_temperature); // Adds the variable label
  sprintf(payload, "%s\"%s\": %s}", payload, VARIABLE_LABEL_4, str_ledstate); // Adds the variable label

  Serial.println("Publishing data to Ubidots Cloud");
  Serial.println(topic);
  Serial.println(payload);
  client.publish(topic, payload);
  client.loop();
  delay(10000);
}
