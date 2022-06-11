//----------- Librerías -----------//

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

//----------- Pines -----------//

#define PREC A0
#define DHTPIN D2
#define DHTTYPE DHT11
#define PIN_TRIG D5
#define PIN_ECHO D6
#define SENSOR D1

//----------- Variables de Sensor -----------//

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate = 0;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
int nivelPrecaucion = 162;
int nivelAlerta = 180;

//----------- Definición de Sensores -----------//

DHT dht(DHTPIN, DHTTYPE, 6);

//----------- Definición de Funciones -----------//

float LevelAgua();
float DHTHumedad();
float DHTTemperatura();
float LevelPrecipitaciones();
float WaterFlow();
String Fecha();
int Alerta();
int NivelPrecaucion();


//----------- Funcíon de apoyo -----------//

void IRAM_ATTR pulseCounter()
{
	pulseCount++;
}

//----------- MQTT CONFIG -----------//
const char *mqtt_server = "flowriver.online";
const int mqtt_port = 1883;
const char *mqtt_user = "ESP8266-1";
const char *mqtt_pass = "flowriver";
const char *root_topic_subscribe = "testtopic";
const char *root_topic_publish = "flowriver/ESP8266-1";


//----------- WIFI -----------//
const char *ssid = "CasaUIS";
const char *password = "YTR1256FGD";

//----------- Variables de Envio -----------//
WiFiClient espClient;
PubSubClient client(espClient);
char msg[125];
long VelocidadAgua = 0;
long Temperatura = 0;
long Humedad = 0;
long Precipitaciones = 0;

//----------- Funciones Wifi -----------//
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void setup_wifi();

//----------- Configuración -----------//

void setup()
{
	Serial.begin(921600);
	pinMode(PIN_TRIG, OUTPUT);
	pinMode(PIN_ECHO, INPUT);
	dht.begin();
	pinMode(SENSOR, INPUT_PULLUP);
	pulseCount = 0;
	flowRate = 0.0;
	flowMilliLitres = 0;
	previousMillis = 0;
	attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
	setup_wifi();
	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);
}

//----------- Loop -----------//

void loop()
{
	if (!client.connected())
	{
		reconnect();
	}

	if (client.connected())
	{
		String str = "{'NivelAgua':" + String(LevelAgua()) + ",'VelAgua':" + String(WaterFlow()) // L/min
					 + ",'Temp':" + String(DHTTemperatura()) + ",'Hum':" + String(DHTHumedad()) + ",'Prec':" + String(LevelPrecipitaciones()) + ",'Fecha':'" + Fecha() + "'}";
		str.toCharArray(msg, 125);
		client.publish(root_topic_publish, msg);
		nivelPrecaucion = NivelPrecaucion();
		nivelAlerta = Alerta();

		if (LevelAgua() > nivelAlerta){	
		delay(9000);//Hay cada 10 segundos
		Serial.println("En estado de Alerta");
		}
		else{
		delay(59000);//cada minuto
		}
		Serial.println(str);
		Serial.println(nivelPrecaucion);
		Serial.println(nivelAlerta);
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
		String clientId = "ESP8266-1";
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

//----------- Nivel del Agua / Ultrasonido -----------//

float LevelAgua()
{
	float tiempo;
	float distancia;

	digitalWrite(PIN_TRIG, LOW); // para generar un pulso limpio ponemos a LOW 4us
	delayMicroseconds(4);

	digitalWrite(PIN_TRIG, HIGH); // generamos Trigger (disparo) de 10us
	delayMicroseconds(10);
	digitalWrite(PIN_TRIG, LOW);

	tiempo = pulseIn(PIN_ECHO, HIGH);
	distancia = 217 - tiempo / 58.3; // Tomamos tamaño maximo del aislante del sensor
	// restamos por la distancia a la fuente

	return 100+rand()%(200-100);
}

//----------- DHT11 Humedad -----------//

float DHTHumedad()
{
	float humedad;
	bool si;
	humedad = dht.readHumidity();
	si = isnan(humedad);
	if (si)
	{
		humedad = 0;
	}
	return 10+rand()%(90-10);
}

//----------- DHT11 Temperatura -----------//

float DHTTemperatura()
{
	float temperatura;
	bool si;
	temperatura = dht.readTemperature();
	si = isnan(temperatura);
	if (si)
	{
		temperatura = 0;
	}
	return 20+rand()%(40-20);
}

//----------- Water Sensor Level -----------//

float LevelPrecipitaciones()
{
	float precipitaciones;
	precipitaciones = analogRead(PREC) / 10;
	return 1+rand()%(10-1);
}

//----------- Water Flow Sensor -----------//

float WaterFlow()
{
	currentMillis = millis();
	if (currentMillis - previousMillis > interval)
	{
		pulse1Sec = pulseCount;
		pulseCount = 0;
		flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
		previousMillis = millis();
		flowMilliLitres = (flowRate / 60) * 1000;
	}
	return 10+rand()%(50-10);
}

//----------- Tiempo -----------//

String Fecha()
{
	String fecha;
	String hora;
	WiFiClient client;
	HTTPClient http;

	http.begin(client, "http://worldtimeapi.org/api/timezone/America/Bogota");
	int httpCode = http.GET();
	if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
	{
		String payload = http.getString();

		int beginS = payload.indexOf("datetime");
		int endS = payload.indexOf("day_of_week");
		fecha = payload.substring(beginS + 11, endS - 16);
	}

	return fecha;
}


int Alerta()
{
	int alerta = 0;
	WiFiClient client;
	HTTPClient http;

	http.begin(client, "http://3.134.188.186:3000/api/estacion/topic/ESP8266-1");
	int httpCode = http.GET();
	if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
	{
		String payload = http.getString();

		int beginS = payload.indexOf("nivelAlerta");
		int endS = payload.indexOf("}]}");
		alerta = payload.substring(beginS + 13, endS).toInt();
	}
	return alerta;
}

int NivelPrecaucion()
{
	int Precaucion = 0;
	WiFiClient client;
	HTTPClient http;

	http.begin(client, "http://3.134.188.186:3000/api/estacion/topic/ESP8266-1");
	int httpCode = http.GET();
	if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
	{
		String payload = http.getString();

		int beginS = payload.indexOf("nivelPrecaucion");
		int endS = payload.indexOf("nivelAlerta");
		Precaucion = payload.substring(beginS + 17, endS-2).toInt();
	}
	return Precaucion;
}
// :)
//Transformación de los datos 
// obj = datos del websocket
//obj = JSON.parse(obj.replaceAll("'",'"'));