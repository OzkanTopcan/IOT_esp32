/********************************
 * Constants and objects
 *******************************/
#include <SPI.h>
#include <WiFi.h>
#include <DHT.h>

#define DEVICE_LABEL "esp_rest"
#define TOKEN "XXXXXXXXX"
#define LED 2
char const * VARIABLE_LABEL = "ioled";
char const * VARIABLE_LABEL_1 = "humidity";
char const * VARIABLE_LABEL_2 = "temperature";

String variable_label  = "ledstate";
String variable_label_1 = "humidity";
String variable_label_2 = "temperature";

char const *SERVER = "industrial.api.ubidots.com";
const int HTTPPORT = 80;
char const *AGENT = "ESP32";
char const *HTTP_VERSION = " HTTP/1.1\r\n";
char const *VERSION = "1.0";
char const *PATH = "/api/v1.6/devices/";

char ssid[] = "XXXXXXXX";
char pass[] = "XXXXXXXX";

// Space to store values to send
char str_humidity[10];
char str_temperature[10];

const int DHTPIN = 4; // Pin where is connected the DHT11
const int DHTTYPE = DHT11; // Type of DHT
DHT dht(DHTPIN, DHTTYPE);

int status = WL_IDLE_STATUS;

/* Reading temperature and humidity */
float humidity = 0.00;
float temperature = 0.00; 
float ledstate = 0;

WiFiClient client;

/********************************
 * Auxiliar Functions
 *******************************/

void printWiFiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void getFromUbidots(char* response) {

  /* Connecting the client */
  client.connect(SERVER, HTTPPORT);

  if (client.connected()) {
    /* Builds the request GET - Please reference this link to know all the request's structures https://ubidots.com/docs/api/ */

    client.print(F("GET "));
    client.print(PATH);    
    client.print(DEVICE_LABEL); 
    client.print(F("/"));
    client.print(VARIABLE_LABEL);
    client.print(F("/lv"));
    client.print(HTTP_VERSION);
    client.print(F("Host: "));
    client.print(SERVER);
    client.print(F("\r\n"));
    client.print(F("User-Agent: "));
    client.print(AGENT);
    client.print(F("/"));
    client.print(VERSION);
    client.print(F("\r\n"));
    client.print(F("X-Auth-Token: "));
    client.print(TOKEN);
    client.print(F("\r\n"));
    client.print(F("Connection: close\r\n"));
    client.print(F("Content-Type: application/json\r\n\r\n"));

    Serial.println(F("Making request to Ubidots:\n"));
    Serial.print(F("GET "));
    Serial.print(PATH);    
    Serial.print(DEVICE_LABEL); 
    Serial.print(F("/"));
    Serial.print(VARIABLE_LABEL);
    Serial.print(F("/lv"));
    Serial.print(HTTP_VERSION);
    Serial.print(F("Host: "));
    Serial.print(SERVER);
    Serial.print(F("\r\n"));
    Serial.print(F("User-Agent: "));
    Serial.print(AGENT);
    Serial.print(F("\r\n"));
    Serial.print(F("X-Auth-Token: "));
    Serial.print(TOKEN);
    Serial.print(F("\r\n"));
    Serial.print("Content-Type: application/json\r\n\r\n");

    waitServer();
    getResponseServer(response);
  }

  else {
    Serial.println("Connection Failed ubidots - Try Again");
    }
}

void postToUbidots(char* response) {

  /* Connecting the client */
  client.connect(SERVER, HTTPPORT);

  if (client.connected()) {
    /* Builds the request POST - Please reference this link to know all the request's structures https://ubidots.com/docs/api/ */
        
  // We now create a URI for the POST request
  String URL = "/api/v1.6/devices/";
         URL += DEVICE_LABEL;
         URL += "/?token=";
         URL += TOKEN;

  // We now create the body to be posted
  String BODY = "{\"";
         BODY += variable_label_2 + "\":" + str_temperature + ", \"";
         BODY += variable_label_1 + "\":" + str_humidity + ", \"";
         BODY += variable_label + "\":" + ledstate;
         BODY += "}";
  Serial.println(BODY);

  int fullSize = BODY.length();
  Serial.println(fullSize);
 
  Serial.println("Requesting URL POST: ");
  Serial.println(URL);

  client.print(F("POST "));
  client.print(URL);
  client.print(F(" HTTP/1.1\r\n"));
  client.print(F("Host: "));
  client.print(SERVER);
  client.print(F("\r\n"));
  client.print(F("Content-Type: application/json\r\n"));
  client.print(F("Content-Length: "));
  client.print(fullSize);
  client.print(F("\r\n"));
  client.print(F("Connection: close\r\n\r\n"));
  client.print(BODY);
  client.print(F("\r\n\r\n"));           
               
    waitServer();
    getResponseServer(response);
  }

  else {
    Serial.println("Connection Failed ubidots - Try Again");
    }
}

void waitServer() {

  int timeout = 0;
  while (!client.available() && timeout < 5000) {
    timeout++;
    delay(1);
    if (timeout >= 5000) {
      Serial.println(F("Error, max timeout reached"));
      break;
    }
  }
}

void getResponseServer(char* response) { 

    /* Reads the response from the server */
    int i = 0;
    sprintf(response, "");
    if (client.available() > 0) {
      while (client.available()) {
        char c = client.read();
        Serial.print(c); // Uncomment this line to visualize the response on the Serial Monitor
        response[i++] = c;
        if (i >= 699){
          break;
        }
      }
    }
    for (int j = i; j < strlen(response) - 1; j++) {
      response[j++] = '\0';
    }
    /* Disconnecting the client */
    client.stop();
}

float parseUbiResponse(char* data, int dstSize=700){
  float error_value = -3.4028235E+8;
  char parsed[20];
  char dst[20];
  int len = strlen(data);  // Length of the answer char array from the server

  for (int i = 0; i < len - 2; i++) {
    if ((data[i] == '\r') && (data[i + 1] == '\n') && (data[i + 2] == '\r') && (data[i + 3] == '\n')) {
      strncpy(parsed, data + i + 4, 20);  // Copies the result to the parsed
      parsed[20] = '\0';
      break;
    }
  }

  /* Extracts the value */
  uint8_t index = 0;

  // Creates pointers to split the value
  char* pch = strchr(parsed, '\n');
  if (pch == NULL) {
    return error_value;
  }

  char* pch2 = strchr(pch + 1, '\n');

  if (pch2 == NULL) {
    return error_value;
  }

  index = (int)(pch2 - pch - 1);

  sprintf(dst, "%s", pch);
  dst[strlen(dst) - 1] = '\0';

  float result = atof(dst);
  return result;
}

/********************************
 * Main Functions
 *******************************/

void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  dht.begin();
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");
  printWiFiStatus();
}

void loop(){

  if (client.connect(SERVER, HTTPPORT)) {
    /* Calls the Ubidots Function */
    char* response = (char *) malloc(sizeof(char) * 700);
    sprintf(response, "");
    
    getFromUbidots(response);

    // Memory space to store the request result
    float results = parseUbiResponse(response);
    Serial.print("Valeur ioled : ");
    Serial.println(results);
    Serial.println();

    if (results == 1) {
      digitalWrite(LED, HIGH);   
      ledstate=0;
    } else {
      digitalWrite(LED, LOW);
      ledstate=1;
    }

    free(response); 
    
    // Wait a few seconds between requests.
    delay(2000);

    /* Reading temperature and humidity */
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    /* test si la lecture du sensor se fait pas */
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    }

    /* 4 is mininum width, 2 is precision; float value is copied onto str_*/
    dtostrf(humidity, 4, 2, str_humidity);
    dtostrf(temperature, 4, 2, str_temperature);
    
    /* Calls the Ubidots Function */
    char* response1 = (char *) malloc(sizeof(char) * 700);
    sprintf(response1, "");

    postToUbidots(response1);
    
    free(response1);
  } 

  else {

    Serial.println("Could not connect to cloud");
    Serial.println("Attemping again in 5 seconds ....");
  }

  delay(5000);
 }
