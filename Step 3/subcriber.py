import paho.mqtt.client as mqtt
import time
import json


# Callback when the client connects to the MQTT broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(subscribe_topic)  # Subscribe to the receive topic
    else:
        print("Connection failed with code {rc}")
        
    # Callback when a message is received from the subscribed topic
def on_message(client, userdata, msg):
    message = str(msg.payload.decode("utf-8"))
    print ("Message received " + "on "+ subscribe_topic + ": "  + message)
    
    #Wrting msg on file
    with open("data_file_new.json", "w") as write_file:
        json.dump(json.loads(message), write_file,indent=4)
        print("Wrote to the file succssfully")
    
    
# Create an MQTT client instance
client = mqtt.Client("PythonSub")

# Set the callback functions
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
broker_address = "test.mosquitto.org"  # broker's address
broker_port = 1883
keepalive = 5
qos = 0


subscribe_topic = input ('Enter the topic to subscribe to: ')
client.connect(broker_address, broker_port, keepalive)

# Start the MQTT loop to handle network traffic
client.loop_start()

# Subscribe loop

try:
    while True:
        time.sleep(6)

except KeyboardInterrupt:
    # Disconnect from the MQTT broker
    pass
client.loop_stop()
client.disconnect()

print("Disconnected from the MQTT broker")
