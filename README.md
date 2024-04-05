[FILE DESCRIPTION]
1) Mining_sensor.ino ==> C script for ESP32 includes reading data from sensors, and publishing on MQTT Server
2) InfluxDataPusher.py ==> Python script includes MQTT Subscriber, InfluxDB support 

To proper use needs:
* MQTT Broker (Mosquitto tested)
* InfluxDB
* Optional GUI (Grafana tested)
  
Our Setup(InfluxDB ==> Grafana):

![image](https://github.com/zakolaczek/Hackathon/assets/149005362/ef3c1228-eaca-4cd2-aa09-d1a8efbe3d49)
