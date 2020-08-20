# CC3200_Azure-IoT-hub-mqtt

Monitoring Acclerometer and Temperature readings in real time on Microsoft Azure IoT hub service using MQTT protocol.


## DESCRIPTION
MQTT is a good fit for embedded WiFi solutions because it is a lightweight protocol. With MQTT, there are 3 main components:

PUBLISHER: In our demo, the LaunchPad will be the publisher, as it will be publishing sensor data under a specific ''Topic''.

BROKER: This is the ''middle man'' who holds on to the data that is being published.
In this example, we are using a publicly available MQTT broker that is suitable for prototyping/demo purposes. 
You can find a complete list of public MQTT brokers here: https://github.com/mqtt/mqtt.github.io/wiki/public_brokers

SUBSCRIBER: In order to subscribe to any data that is being sent by a PUBLISHER, the SUBSCRIBER must be connected to the same BROKER & subscribed to the same topic as the PUBLISHER.
As long as these 2 conditions are met, the SUBSCRIBER will be able to receive messages from the PUBLISHER.

## Hardware Used :

    Texas Instrument CC3200 launchpad
    GY-521 Mpu6050 Module Accelerometer
    LM-35 temperature Sensor

