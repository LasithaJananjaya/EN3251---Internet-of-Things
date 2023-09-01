#jason introduction 2
#Read sensor data from file
#Write received data to a file
read_file_name = "sensor.json"
write_file_name = "sensor_received.json"
import json

with open(read_file_name) as json_file:
            sensor_out= json.load(json_file)
            
print ("sensor_out = ", sensor_out)
print ("sensor_out is a ", type(sensor_out)) #type <class 'dict'>
print("Temperature =", sensor_out["Temperature"])

#At sender (Encoding)
data_out=json.dumps(sensor_out) #encode object to JSON
print ("\nConverting to JSON\n")
print ("data type is ", type (data_out)) #type <class 'str'>
print("data_out = ", data_out)



#At receiver (Decoding)
print ("\nReceived Data\n")
data_in = data_out
print ("\ndata in type is ", type(data_in))
print ("data in = ", data_in)

sensor_in=json.loads(data_in) #convert incoming JSON to object
print("\nsensor_in is a ", type(sensor_in))
print ("\nHumidity = ", sensor_in["Humidity"])


# Write data to JSON file

with open(write_file_name, 'w') as json_file:
    json.dump(sensor_in, json_file, indent=4)  # The 'indent' parameter adds pretty formatting
print("Data has been written to", write_file_name)


      


