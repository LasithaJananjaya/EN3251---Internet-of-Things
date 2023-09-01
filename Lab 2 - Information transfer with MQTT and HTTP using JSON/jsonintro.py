#jason introduction

import json
#At sender
sensor_out = {"Temperature": 20, "Humidity": 25, "Pressure": 40 }
print (sensor_out)
print ("sensor_out is a ", type(sensor_out))
print("Temperature =", sensor_out["Temperature"])

data_out=json.dumps(sensor_out) #encode object to JSON
print ("\nConverting to JSON\n")
print ("data type is ", type (data_out))
print("data out = ", data_out)

#At receiver
print ("\nReceived Data\n")
data_in = data_out
print ("\ndata in type is ", type(data_in))
print ("data in = ", data_in)

sensor_in=json.loads(data_in) #convert incoming JSON to object
print("\nsensor_in is a ", type(sensor_in))
print ("\nHumidity = ", sensor_in["Humidity"])
       




      


