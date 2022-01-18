#include <Arduino.h>
#include <Colors.h>
#include <IoTicosSplitter.h>
#include <ESP8266WiFi.h>       //#include <WiFi.h>
#include <ESP8266HTTPClient.h> //<HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

//----------- MQTT CONFIG -----------//
const char *mqtt_server = "ioticos.org";
const int mqtt_port = 1883;
const char *mqtt_user = "";
const char *mqtt_pass = "";
const char *root_topic_subscribe = "testtopic";
const char *root_topic_publish = "testtopic";


//----------- WIFI -----------//
const char* ssid = "Callen";
const char* password =  "algoalgo";



//----------- Globales -----------//
WiFiClient espClient;
PubSubClient client(espClient);
char msg[25];
long count=0;


//----------- Funciones -----------//
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();


void setup()
{
  Serial.begin(921600);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  
  count++;

  if (!client.connected()) {
		reconnect();
	}

  if (client.connected()){
    String str = "La cuenta es -> " + String(count);
    str.toCharArray(msg,25);
    client.publish(root_topic_publish,msg);
    delay(300);
  }
  client.loop();
}

//----------- Conexión Wifi -----------//

void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ssid: ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}


//----------- Conexión MQTT -----------//

void reconnect() {

	while (!client.connected()) {
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "IOTICOS_H_W_";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
			Serial.println("Conectado!");
			// Nos suscribimos
			if(client.subscribe(root_topic_subscribe)){
        Serial.println("Suscripcion ok");
      }else{
        Serial.println("fallo Suscripciión");
      }
		} else {
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");
			delay(5000);
		}
	}
}

//----------- Callback -----------//

void callback(char* topic, byte* payload, unsigned int length){
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("Mensaje -> " + incoming);

}