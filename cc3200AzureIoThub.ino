/*
 Basic MQTT example 
 
  - connects to an MQTT server
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic"
*/

#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BMA222.h>
BMA222 mySensor;
int Xdata;
int Ydata;
int Zdata;
char AccX[10]="";
char AccY[10]="";
char AccZ[10]="";
char Accl_read[50];

// your network name also called SSID
char ssid[] = "Datasemantics";
char password[] = "D@ta#sem@nticz";
// MQTTServer to use
char server[] = "192.168.20.22";

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
  Serial.begin(115200);
   mySensor.begin();
  // Start Ethernet with the build in MAC Address
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
      if(client.subscribe("inTopic")) {
        Serial.println("Subscription successfull");
      }
    }
  }

  int8_t Xdata = mySensor.readXData();
  Serial.print("X: ");
  Serial.print(Xdata);
  dtostrf(Xdata, 3, 1, AccX);
  Ydata = mySensor.readYData();
  Serial.print(" Y: ");
  Serial.print(Ydata);
  dtostrf(Ydata, 3, 1, AccY);
  Zdata = mySensor.readZData();
  Serial.print(" Z: ");
  Serial.println(Zdata);

  dtostrf(Zdata, 3, 1, AccZ);

  
  strcat(Accl_read,AccX);
  strcat(Accl_read,AccY);
  strcat(Accl_read,AccZ); 

  if(client.publish("cc/sensorData",Accl_read)) {
    Serial.println("Publish success");
  } else {
    Serial.println("Publish failed");
  } 
  // Check if any message were received
  // on the topic we subsrcived to
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
