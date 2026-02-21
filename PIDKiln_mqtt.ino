#include <PubSubClient.h>
//IPAddress mqtt_server(Prefs[PRF_SYSLOG_SRV].value.str);
//const char* mqtt_server = "192.168.0.52";
IPAddress mqtt_server;

float temperature = 0;
long lastMsg = 0;
char msg[20];
char msg2[20];

WiFiClient espClient;
PubSubClient client(espClient);

#define TEMP_TOPIC    "PIDKiln/temp"
#define ERROR_TOPIC    "PIDKiln/error"

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);
 
  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  /*Serial.println();
   //we got '1' -> on 
  if ((char)payload[0] == '1') {
    digitalWrite(led, HIGH); 
  } else {
    // we got '0' -> on 
    digitalWrite(led, LOW);
  }*/
 
}

void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    //Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "ESP32Client";
    /* connect now */
    if (client.connect(clientId.c_str())) {
      //Serial.println("connected");
      /* subscribe topic with default QoS 0*/
      //client.subscribe(LED_TOPIC);
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(6000);
    }
  }
}

void MQTT_Loop(void * parameter) {  
  for(;;){
    if (!client.connected()) {
      mqttconnect();
    }
  /* we measure temperature every 5 secs
  we count until 5 secs reached to avoid blocking program if using delay()*/
    long now = millis();
    if (now - lastMsg > 5000) {
      lastMsg = now;      
      temperature = kiln_temp;//dht.readTemperature();
      if (!isnan(temperature)) {
        snprintf (msg, 20, "%.lf", temperature);        
        /* publish the message */        
        client.publish(TEMP_TOPIC, msg);        
      }
      snprintf (msg2, 20, "%d", TempA_errors);
      client.publish(ERROR_TOPIC, msg2);
    }
  }
}

void setup_mqtt() {
  /* configure the MQTT server with IPaddress and port */  
  mqtt_server.fromString(Prefs[PRF_SYSLOG_SRV].value.str);
  client.setServer(mqtt_server, 1883);
  client.setCallback(receivedCallback);
  xTaskCreatePinnedToCore(
//  xTaskCreate(
              MQTT_Loop,       /* Task function. */
              "mqtt_loop",     /* String with name of task. */
              8192,             /* Stack size in bytes. */  
              NULL,             /* Parameter passed as input of the task */
              1,                /* Priority of the task. */
              NULL,1);            /* Task handle. */
 
}
