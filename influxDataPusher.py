import influxdb_client, os, time
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
import paho.mqtt.client as mqtt
import time
from influxdb_client.rest import ApiException

#Save subscribed message
def on_message(client, userdata, message):
    #Decoding
    topic = message.topic
    payload = str(message.payload.decode("utf-8"))
    #Saving to dictionary
    output[topic] = payload


#Main logic with universal parameters
def main(topics, client_name, host_adress):


    #Connection with MQTT Broker
    broker = host_adress
    client = mqtt.Client(client_name)
    isError = True
    #Server issue handling 
    while isError:
        try:
            client.connect(broker)
            isError = False
        except TimeoutError: 
            #If can't connect to host, terminate program
            print("Your host adress is not available for now! Check your privacy settings. Trying again in 5 seconds")
            time.sleep(5)
        except ConnectionRefusedError:
            #If host is not available, terminate program
            print("Server is shutdown, Trying again in 5 seconds")
            time.sleep(5)
    #Client Loop Start
    client.loop_start()
    for topic in topics:
        client.subscribe(topic)
        client.on_message = on_message  
    #Client Loop End 

    #Output displaying with refresh_rate (in seconds)
    while True:
        print(output)

        pushForAllIn(output)
        #refreshing after set time
        time.sleep(refresh_rate)

#Push data to influxdb database
def pushData(token, org, url, bucket, measurement, field, tagname, tagvalue,  value):
    if token == 0:
       #in case 0 use default token
       token = defaultToken

    if org == 0:
       #in cas 0 use default org
       org = defaultOrg

    if url == 0:
       #in case 0 use default url
       url = defaultUrl

    if bucket == 0:
       #in case 0 use default bucket
       bucket = defaultBucket

    if measurement == 0:
       #in case 0 use default measurement
       measurement = defaultMeasurement

    if field == 0:
       #in case 0 use default field
       field = defaultField
    
    #Establish connection to InfluxDB server
    write_client = influxdb_client.InfluxDBClient(url=url, token=token, org=org)
    write_api = write_client.write_api(write_options=SYNCHRONOUS)
    point = (Point(measurement).tag(tagname, tagvalue).field(field, value))

    #catch errors while pushing to influxDB
    try :
        write_api.write(bucket=bucket, org="none", record=point)
    except Exception as w:
        #in case error while pushing inform the user and print error and exit program
        print(f"Error occured trying to push to influxDB wrong Bucket, Token Tag, Field, Org, URL or type of variable. Error message:\n{w}")
        exit()

def pushForAllIn(output):
    
    #for all values in recieved mqtt values push to InfluxDB
    for topic in output.keys():
        #try if provided value is int
        try:
            value = int(float(output[topic]))
            pushData(0,0,0,0,topic,0,topic,topic,value)
        except ValueError:
            #if recieved value is not int disable pushing and print error message
            print(f"Recieved value is not int, for {output}:{output[topic]}")
        
#defining default values to use for pushing to InfluxDB
defaultToken = "BoUJ4_vx56CEnW6FNT1jvr1i1qGnUb4EEq9QwKi4zCmh_wRGFKAH4HahOYUGTQ_Qe7Uo1VYkOQOHIJJhpNkbWw=="
defaultOrg = "none"
defaultUrl = "http://localhost:8086"
defaultBucket = "DeviceData"
defaultMeasurement="measurement1"
defaultField = "field2"

#parameters for mqtt subscribing

#topics to get value of
topics_to_subscribe = ["MQ-2", "MQ-7","ALARM","TEMPERATURA","WILGOTNOSC"]
#name of the client
name_client = "Subscriber"
#adress of mqtt broker
adress = "172.31.99.244"
#rate in which the recieved data is refreshed (in seconds)
refresh_rate = 1
#output dictionary to store recieved data in
output = {}
#Begin
main(topics_to_subscribe, name_client, adress)

