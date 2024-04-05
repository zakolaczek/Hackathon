#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>

/*Initalize MQTT publisher */
WiFiClient esp_client;
PubSubClient client(esp_client);
const char* mqtt_server = "172.31.99.244"; 
long lastMsg = 0;
char msg[50];
int value = 0;

DHT dht;

/*MQ-2 sensor analog pin
  function of MQ-2 sensor is to detect methane and gas */
#define GAS_PIN 33
const char* message_topic_MQ_2 = "MQ-2";

#define DHT11_PIN 14

/*MQ-7 sensor analog pin
  function of MQ-7 sensor is to detect carbon monoxide */
#define CO_PIN 27
const char* message_topic_MQ_7 = "MQ-7";

/* Tilt switch function is to detect, when worker is down */
#define TILT_PIN 18

/*Alarm buzzer function is to generate huge sound in crisis situation */
#define BUZZER_PIN 2
const char* message_topic_alarm = "ALARM";

/* Confirmation button that miner is not endangered */
#define CONFIRM_PIN 35
const char* message_topic_confirm = "CONFIRM";

/* check how much seconds miner is down */
int seconds_miner_down = 0;
bool confirm_miner_fine = 0;

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("MQ-2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* function to connect with wifi */
void connect_to_wifi(const char* ssid, const char* password)
{

  WiFi.begin(ssid, password);
  int seconds = 0;

  while(WiFi.status() != WL_CONNECTED)
  {
    seconds++;
    delay(1000);

    /* If there are more than 20 seconds , esp send warning message */
    if(seconds > 30)
    {
      Serial.println("Can't connect to WiFI, please run again code or check that your's password and ssid are correct");
      break;
    }
  }
} 

/* sending MQTT messages */
bool send_mqtt_message(const char* topic, int number)
{
  char number_string[8];
  dtostrf(number, 1, 2, number_string);
  client.publish(topic, number_string);

} 

bool start_alarm_miner(int seconds)
{
  if(seconds > 5 && !confirm_miner_fine)
  {
      digitalWrite(BUZZER_PIN, HIGH);
      return true;
  }

  digitalWrite(BUZZER_PIN, LOW);

  if(digitalRead(TILT_PIN))
  {
    confirm_miner_fine = 0;
  }

  return false;

}

void check_confirm_miner()
{
  if(digitalRead(CONFIRM_PIN))
  {
    digitalWrite(BUZZER_PIN, LOW);
    confirm_miner_fine = 1;
  }

}
void setup() {
  Serial.begin(9600);

  const char* ssid = "Hackathon";
  const char* password = "Hackathon24";
  connect_to_wifi(ssid, password);
  
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);

  //set pins to read values from sensors
  pinMode(GAS_PIN, INPUT);
  pinMode(CO_PIN, INPUT);
  pinMode(TILT_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CONFIRM_PIN, INPUT);

  dht.setup(DHT11_PIN);

  digitalWrite(TILT_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

}

void loop() {
  //get values from sensors
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int carbon_monoxide = analogRead(CO_PIN);
  int methan_gas = analogRead(GAS_PIN);
  int miner_down = !digitalRead(TILT_PIN);
  int humidity = dht.getHumidity();
  int temperature = dht.getTemperature();

  Serial.print("Value from methan sensor: ");
  Serial.println(methan_gas);

  Serial.print("Value from CO sensor:");
  Serial.println(carbon_monoxide);

  Serial.print("Is miner down: ");
  Serial.println(miner_down);
  
  Serial.print("\n\n");

  if(miner_down)
  {
    seconds_miner_down++;
  }

  else
  {
    seconds_miner_down = 0;
  }

  /*check for alarm */
  bool is_alarm = start_alarm_miner(seconds_miner_down);

  /*check for confirming that everything is good */
  if(is_alarm);
  {
    check_confirm_miner();
  }

  long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
    
    char tempString[8];
    dtostrf(methan_gas, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("MQ-2", tempString);

    dtostrf(carbon_monoxide, 1, 2, tempString);
    Serial.print("Czad: ");
    Serial.println(tempString);
    client.publish("MQ-7", tempString);

    dtostrf(digitalRead(BUZZER_PIN), 1, 2, tempString);
    Serial.print("ALARM: ");
    Serial.println(tempString);
    client.publish("ALARM", tempString);

    dtostrf(temperature, 1, 2, tempString);
    Serial.print("TEMPERATURA: ");
    Serial.println(tempString);
    client.publish("TEMPERATURA", tempString);

    dtostrf(humidity, 1, 2, tempString);
    Serial.print("WILGOTNOSC: ");
    Serial.println(tempString);
    client.publish("WILGOTNOSC", tempString);
  }

  delay(1000);
}
