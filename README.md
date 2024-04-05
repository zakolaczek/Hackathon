[FILE DESCRIPTION]
1) Mining_sensor.ino ==> C script for ESP32 includes reading data from sensors, and publishing on MQTT Server
2) InfluxDataPusher.py ==> Python script includes MQTT Subscriber, InfluxDB support 

To proper use needs:
* MQTT Broker (Mosquitto tested)
* InfluxDB
* Optional GUI (Grafana tested)
