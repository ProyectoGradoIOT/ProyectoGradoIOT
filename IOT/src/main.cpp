#include <Arduino.h>
#include <wire.h>
#include <Colors.h>
#include <IoTicosSplitter.h>
#include <ESP8266WiFi.h>	   //#include <WiFi.h>
#include <ESP8266HTTPClient.h> //<HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define PREC A0
#define DHTPIN D2
#define DHTTYPE DHT11
#define PIN_TRIG D5
#define PIN_ECHO D6

DHT dht(DHTPIN, DHTTYPE,6);

float tiempo;
float distancia;
float humedad;
float temperatura;
float precipitaciones;

float LevelAgua();
float DHTHumedad();
float DHTTemperatura();
float LevelPrecipitaciones();


//----------- MQTT CONFIG -----------//
const char *mqtt_server = "flowriver.online";
const int mqtt_port = 1883;
const char *mqtt_user = "ESP8266-2";
const char *mqtt_pass = "flowriver";
const char *root_topic_subscribe = "testtopic";
const char *root_topic_publish = "flowriver/Esp8266-2";

//----------- WIFI -----------//
const char *ssid = "CASAUIS_2";
const char *password = "a1b2c3d4/casauis";

//----------- Globales -----------//
WiFiClient espClient;
PubSubClient client(espClient);
char msg[125];
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
	pinMode(PIN_TRIG, OUTPUT);
	pinMode(PIN_ECHO, INPUT);
	dht.begin();
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
		VelocidadAgua = 45+rand()%(65-45);
		String str = "{'NivelAgua':" + String(LevelAgua())
		+ ",'VelocidadAgua':" + String(VelocidadAgua) 
		+ ",'Temperatura':" + String(DHTTemperatura())
		+ ",'Humedad':" + String(DHTHumedad())
		+ ",'Precipitaciones':" + String(LevelPrecipitaciones()) + "}";
		str.toCharArray(msg,125);
		client.publish(root_topic_publish,msg);
		delay(1000);
		Serial.println(str);
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
		String clientId = "ESP8266-2";
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

float LevelAgua()
{
	digitalWrite(PIN_TRIG, LOW); // para generar un pulso limpio ponemos a LOW 4us
	delayMicroseconds(4);

	digitalWrite(PIN_TRIG, HIGH); // generamos Trigger (disparo) de 10us
	delayMicroseconds(10);
	digitalWrite(PIN_TRIG, LOW);

	tiempo = pulseIn(PIN_ECHO, HIGH);
	distancia = 217 - tiempo / 58.3;

	return distancia;
}

float DHTHumedad()
{
	humedad = dht.readHumidity();
	return humedad;
}

float DHTTemperatura()
{
	temperatura = dht.readTemperature();
	return temperatura;
}

float LevelPrecipitaciones(){
	precipitaciones = analogRead(PREC)/10;
	return precipitaciones;
}
