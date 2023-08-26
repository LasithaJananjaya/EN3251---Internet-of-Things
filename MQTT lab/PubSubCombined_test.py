import paho.mqtt.client as mqtt
import time

# Callback when the client connects to the MQTT broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker\n")
        client.subscribe(subscribe_topic, qos = 0)  # Subscribe to the receive topic
    else:
        print("Connection failed with code {rc}")

# Callback when a message is received from the subscribed topic
def on_message(client, userdata, msg):
    print ("Message received " + "on "+ subscribe_topic + ": "  + str(msg.payload.decode("utf-8")))

# Create an MQTT client instance
client = mqtt.Client("") #Replace PythonHuub. you can keep it blank here, then the client will use a unique ID. Otherwise use your own name.

# Set the callback function
client.on_connect = on_connect
client.on_message = on_message

broker_address = "test.mosquitto.org"  # broker's address
broker_port = 1883
keepalive = 5
qos = 0
publish_topic = "protocolprotopic"

subscribe_topic = input ('Enter the topic to subscribe to: ')

# Connect to the MQTT broker
client.connect(broker_address, broker_port, keepalive)

# Start the MQTT loop to handle network traffic
client.loop_start()

# Publish loop

try:
    while True:
        # Publish a message to the send topic
        
        value = input('Enter the message: ')
        client.publish(publish_topic,value)
        print(f"Published message '{value}' to topic '{publish_topic}'\n")
        
        # Wait for a moment to simulate some client activity
        time.sleep(6)

except KeyboardInterrupt:
    # Disconnect from the MQTT broker
    pass
client.loop_stop()
client.disconnect()

print("Disconnected from the MQTT broker")
