import Adafruit_DHT
import iothub_client
from iothub_client import *
from iothub_client_args import *
import sys
import time
from time import gmtime, strftime

sensor = Adafruit_DHT.DHT11
pin = 4

message_timeout = 10000

receive_context = 0
avg_wind_speed = 10.0
message_count = 5
received_count = 0

# global counters
receive_callbacks = 0
send_callbacks = 0
blob_callbacks = 0

# chose HTTP, AMQP or MQTT as transport protocol
protocol = IoTHubTransportProvider.AMQP
connection_string = "HostName=IoThub-ds.azure-devices.net;DeviceId=RaspberryPi;SharedAccessKey=qGJPJkZlif3HcAV5Y/FTiJ5wnvcoGg6rFl/GmvWFRU8="



def iothub_client_init():
    # prepare iothub client
    iotHubClient = IoTHubClient(connection_string, protocol)
    if iotHubClient.protocol == IoTHubTransportProvider.HTTP:
        iotHubClient.set_option("timeout", timeout)
        iotHubClient.set_option("MinimumPollingTime", minimum_polling_time)
    # set the time until a message times out
    iotHubClient.set_option("messageTimeout", message_timeout)
    # some embedded platforms need certificate information
    # set_certificates(iotHubClient)
    # to enable MQTT logging set to 1
    if iotHubClient.protocol == IoTHubTransportProvider.MQTT:
        iotHubClient.set_option("logtrace", 0)
    iotHubClient.set_message_callback(
        receive_message_callback, receive_context)
    return iotHubClient

def receive_message_callback(message, counter):
    global receive_callbacks
    buffer = message.get_bytearray()
    size = len(buffer)
    print("Received Message [%d]:" % counter)
    print("    Data: <<<%s>>> & Size=%d" % (buffer[:size].decode('utf-8'), size))
    map_properties = message.properties()
    key_value_pair = map_properties.get_internals()
    print("    Properties: %s" % key_value_pair)
    counter += 1
    receive_callbacks += 1
    print("    Total calls received: %d" % receive_callbacks)
    return IoTHubMessageDispositionResult.ACCEPTED


def send_confirmation_callback(message, result, user_context):
    global send_callbacks
    print(
        "Confirmation[%d] received for message with result = %s" %
        (user_context, result))
    map_properties = message.properties()
    print("    message_id: %s" % message.message_id)
    print("    correlation_id: %s" % message.correlation_id)
    key_value_pair = map_properties.get_internals()
    print("    Properties: %s" % key_value_pair)
    send_callbacks += 1
    print("    Total calls confirmed: %d" % send_callbacks)



if __name__ == '__main__':
    print(strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime()))
    print('Attempting a read of DHT11')
    humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
      
    #Loop until a reading has been made as the DHT11 sometimes failes to read
    while humidity is None and temperature is None:
        time.sleep(2)
        print('Attempting a read of DHT11')
  humidity, temperature = Adafruit_DHT.read_retry(sensor, pin)
  print(humidity, temprature)
  
    print('Temp={0:0.1f}*C  Humidity={1:0.1f}%'.format(temperature, humidity))
    
    #Initate the IoT hub
    iotHubClient = iothub_client_init()

    (connection_string, protocol) = get_iothub_opt(sys.argv[1:], connection_string, protocol)
    
    #Send the reading message to the IoT hub
    msg_txt = "{\"DeviceRef\": \"PiDevice1\",\"Temp\": %.2f, \"Humidity\": %.2f}"
    msg_txt_formatted = msg_txt % (temperature,humidity)
    print("JSON payload = " + msg_txt_formatted)

    message = IoTHubMessage(msg_txt_formatted)
    
    i = 1  #1 message per exe so static 1
    
    message.message_id = "message_%d" % i
    message.correlation_id = "correlation_%d" % i
    iotHubClient.send_event_async(message, send_confirmation_callback,i)
   
    #The following is required to ensure the program doesn't end before the message is sent 
    #as will result in a DESTROIED message status
    n = 0
    while n < 3:
        status = iotHubClient.get_send_status()
        print("Send status: %s" % status)
        time.sleep(10)
        n += 1
