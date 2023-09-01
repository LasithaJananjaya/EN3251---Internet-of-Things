import paho.mqtt.client as mqtt
import time
import json

# Callback when the client connects to the MQTT broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker\n")
    else:
        print("Connection failed with code {rc}")
        

# Create an MQTT client instance
client = mqtt.Client("")

# Set the callback function
client.on_connect = on_connect

broker_address = "test.mosquitto.org"  # broker's address
broker_port = 1883
keepalive = 5
qos = 0
publish_topic = "IoT"

# Connect to the MQTT broker
client.connect(broker_address, broker_port, keepalive)

# Start the MQTT loop to handle network traffic
client.loop_start()


#Reading from json file
with open("data_file.json", "r") as read_file:
    doc_json = json.dumps(json.load(read_file))
    

    
#print(type(doc_json))
    

# Publish object literal

value = doc_json
client.publish(publish_topic,value,qos)
print(f"Published message '{value}' to topic '{publish_topic}'\n")

# Wait for a moment to simulate some client activity
time.sleep(6)

print("Disconnected from the MQTT broker")

input("Press ENTER to exit")


