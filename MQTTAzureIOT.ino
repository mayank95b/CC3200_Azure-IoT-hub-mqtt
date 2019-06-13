/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

//#ifndef __CC3200R1M1RGC__
// Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "sha256.h"
#include "Base64.h"
//Temperature sensor TMP006 related headers
#include <Wire.h>
#include "Adafruit_TMP006.h"
#define USE_USCI_B1 
//TI CC3220 TMP006 I2C addres is 0x41 : 
//Reference : http://www.ti.com.cn/cn/lit/ug/swru372b/swru372b.pdf / http://blog.gaku.net/cc3200-launchpad/
Adafruit_TMP006 tmp006(0x41);

//--------------------------------------------------------------------------------------------------------------------------
// your network name also called SSID
char ssid[] = "Datasemantics";
char password[] = "D@ta#sem@nticz";
// MQTTServer to use
char server[] = "iot.eclipse.org";

//---------------------------------------------------------------------------------------------------------------------------------------------
// START: Azure IoT Hub settings

//Azure IoT Hub connection string
String IoTHubConnectionString = "HostName=IoTTeam-IoThub.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=6plnHqHGdkT7/sEHoRNzRwxbOPtKONvkOHm6ab4Be1c=";
//String IoTHubConnectionString = "HostName=youriothubname.azure-devices.net;DeviceId=TIcc3200-a;SharedAccessKey=8A0K1er45k5l3achAmda7UvVh6s4L7HTBdt/RnAdYuk=";

boolean isConnectedToIoTHub = false;
const char* IOT_HUB_END_POINT = "/messages/events?api-version=2016-02-03";
char* HOST;
char* DEVICE_ID;
char* KEY;
String strSasToken;
// END: Azure IoT Hub settings
//---------------------------------------------------------------------------------------------------------------------------------------

String dataToSend; //String variable which will be containing the JSON data to send

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received message for topic ");
  Serial.print(topic);
  Serial.print("with length ");
  Serial.println(length);
  Serial.println("Message:");
  Serial.write(payload, length);
  Serial.println();
}

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void setup()
{
   //======================================================================================================
 // PART 1 : Initialize serial
 //======================================================================================================
    Serial.begin(115200);

  //======================================================================================================
 // PART 2 : Initialize TMP006 builtin sensor
 //======================================================================================================
 // Initalizes the TMP006 for operation and for I2C communication
  Serial.print("\nInitializing TMP006 sensor ...  ");
  if (!tmp006.begin(TMP006_CFG_8SAMPLE)) {
    Serial.println("\nNo sensor found");
    while(1);
  }
  Serial.println("OK");

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  // Print the WiFi status.
  printWifiStatus();
}

void loop()
{
  // Reconnect if the connection was lost
  if (!client.connected()) {
    Serial.println("Disconnected. Reconnecting....");

    if(!client.connect("energiaClient")) {
      Serial.println("Connection failed");
    } else {
      Serial.println("Connection success");
      if(client.subscribe("cc/temperature")) {
        Serial.println("Subscription successfull");
      }
    }
  }
  
  if(client.publish("cc/temperature","hello")) {
    Serial.println("Publish success");
  } else {
    Serial.println("Publish failed");
  }
 
  // Check if any message were received
  // on the topic we subsrcived 

 //float objt = tmp006.readObjTempC();
  //Serial.print("Object Temperature: "); Serial.print(objt); Serial.println("*C");
  float diet = tmp006.readDieTempC();
  Serial.print("Die Temperature: "); Serial.print(diet); Serial.println("*C");  
  
  // START: JSON data to send  
  char outstr[15]; //buffer to use in float to char array conversion
  dtostrf(diet,4, 1, outstr);  
  dataToSend = "{temperature: '" + (String)outstr + "'}";
  // END: JSON data to send
  
  Serial.println("\Sending data to Azure IoT Hub...");
  client.poll();
  delay(1000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//-----------------------------------------------------------------------------------------------------------------------------------
//http://hardwarefun.com/tutorials/url-encoding-in-arduino
String urlEncode(const char* msg)
{
    const char *hex = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
                || ('A' <= *msg && *msg <= 'Z')
                || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    return encodedMsg;
}

// http://arduino.stackexchange.com/questions/1013/how-do-i-split-an-incoming-string
String splitStringByIndex(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1  };  
  int maxIndex = data.length()-1;
  
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
      found++;
      strIndex[0] = strIndex[1]+1;
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


const char *GetValue(const char* value){
  char *temp = new char[strlen(value) + 1];
  strcpy(temp, value);
  return temp;
}

const char *GetStringValue(String value){
  int len = value.length() + 1;
  char *temp = new char[len];
  value.toCharArray(temp, len);
  return temp;
}

String createIotHubSasToken(char *key, String url){
  unsigned int sasExpiryDate = 1737504000;  // Expires Wed, 22 Jan 2025 00:00:00 GMT 
  String stringToSign = url + "\n" + sasExpiryDate;

  // START: Create signature
  // https://raw.githubusercontent.com/adamvr/arduino-base64/master/examples/base64/base64.ino
  
  int keyLength = strlen(key);
  
  int decodedKeyLength = base64_dec_len(key, keyLength);
  char decodedKey[decodedKeyLength];  //allocate char array big enough for the base64 decoded key
  
  base64_decode(decodedKey, key, keyLength);  //decode key
  
  Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
  Sha256.print(stringToSign);  
  char* sign = (char*) Sha256.resultHmac();
  // END: Create signature
  
  // START: Get base64 of signature
  int encodedSignLen = base64_enc_len(HASH_LENGTH);
  char encodedSign[encodedSignLen];
  base64_encode(encodedSign, sign, HASH_LENGTH); 
  
  // SharedAccessSignature
  return "sr=" + url + "&sig="+ urlEncode(encodedSign) + "&se=" + sasExpiryDate;
  // END: create SAS  
}
