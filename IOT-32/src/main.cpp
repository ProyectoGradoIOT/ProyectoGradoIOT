#include <Arduino.h>
#include <Colors.h>
#include <IoTicosSplitter.h>
#include <WiFi.h>	   //#include <WiFi.h>
#include <HTTPClient.h> //<HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <stdlib.h>

//----------- MQTT CONFIG -----------//
const char *mqtt_server = "flowriver.online";
const int mqtt_port = 1883;
const char *mqtt_user = "ESP32-CAM";
const char *mqtt_pass = "flowriver";
const char *root_topic_subscribe = "testtopic";
const char *root_topic_publish = "flowriver/ESP32-CAM";

//----------- WIFI -----------//
const char *ssid = "CASAUIS";
const char *password = "a1b2c3d4/casauis";

//----------- Globales -----------//
WiFiClient espClient;
PubSubClient client(espClient);
char msg[125];
long NivelAgua = 0;
long VelocidadAgua = 0;
long Temperatura = 0;
long Humedad = 0;
long Precipitaciones = 0;


//----------- Funciones -----------//
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void setup_wifi();

void setup()
{
	Serial.begin(921600);
	setup_wifi();
	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);
}

void loop()
{

	if (!client.connected())
	{
		reconnect();
	}
	
	  if (client.connected()){
		NivelAgua = 1+rand()%(5-1);
		VelocidadAgua = 45+rand()%(65-45);
		Temperatura =19+rand()%(29-19);
		Humedad =20+rand()%(100-20);
		Precipitaciones = 0+rand()%(10-0);
		String str = "{'NivelAgua':" + String(NivelAgua)
		+ ",'VelocidadAgua':" + String(VelocidadAgua) 
		+ ",'Temperatura':" + String(Temperatura)
		+ ",'Humedad':" + String(Humedad)
		+ ",'Precipitaciones':" + String(Precipitaciones) + "}";
		str.toCharArray(msg,125);
		client.publish(root_topic_publish,msg);
		delay(60000);
	  }

	client.loop();
}

//----------- Conexión Wifi -----------//

void setup_wifi()
{

	delay(10);

	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ssid: ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}

//----------- Conexión MQTT -----------//

void reconnect()
{

	while (!client.connected())
	{
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "ESP32-CAM";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
		{
			Serial.println("Conectado!");
			// Nos suscribimos
			if (client.subscribe(root_topic_subscribe))
			{
				Serial.println("Suscripcion ok");
			}
			else
			{
				Serial.println("fallo Suscripciión");
			}
		}
		else
		{
			Serial.print("falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentamos de nuevo en 5 segundos");
			delay(5000);
		}
	}
}

//----------- Callback -----------//

void callback(char *topic, byte *payload, unsigned int length)
{
	String incoming = "";
	Serial.print("Desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++)
	{
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("-> " + incoming);
}