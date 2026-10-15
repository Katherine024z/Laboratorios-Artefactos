#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

//const char* WIFI_SSID = "ARTEFACTOS";
//const char* WIFI_PASSWORD = "s^3sQ#9teskVgnb";
const char* WIFI_SSID = "GalaxyA21sEDD0";
const char* WIFI_PASSWORD = "Lkjo8612";


// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================

const char* MQTT_SERVER = "10.120.30.183";
const int MQTT_PORT = 1883;

// Topics utilizados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";


// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

// Definir los pines y objetos necesarios para los sensores.
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
const int PIN_LDR = 32;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;

unsigned long ultimoEnvio = 0;

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String(
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      HEX
    );

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}


// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {

  /*
      1. Obtener la temperatura.
      2. Obtener la humedad.
      3. Obtener la lectura del sensor de iluminación.
      4. Preparar los valores para publicarlos mediante MQTT.
      5. Publicar cada dato en su topic correspondiente.
      6. Mostrar las mediciones en el Monitor Serial.
  */

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  int luz = analogRead(PIN_LDR);

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer el sensor DHT11");
    return;
  }

  // Preparar los valores para publicarlos mediante MQTT.
  char valorTemperatura[16];
  char valorHumedad[16];
  char valorLuz[16];

  snprintf(valorTemperatura, sizeof(valorTemperatura),
           "%.2f", temperatura);

  snprintf(valorHumedad, sizeof(valorHumedad),
           "%.2f", humedad);

  snprintf(valorLuz, sizeof(valorLuz), "%d", luz);

  // Publicar los datos en sus topics correspondientes.
  mqttClient.publish(TOPIC_TEMPERATURA, valorTemperatura);
  mqttClient.publish(TOPIC_HUMEDAD, valorHumedad);
  mqttClient.publish(TOPIC_LUZ, valorLuz);

  // Mostrar las mediciones en el Monitor Serial.
  Serial.println("----- SENSORES -----");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.print("Luz: ");
  Serial.println(luz);

  Serial.println("--------------------");
}


void setup() {

  Serial.begin(115200);

  // Inicializar los sensores y configurar los pines necesarios.
  dht.begin();
  pinMode(PIN_LDR, INPUT);

  conectarWiFi();

  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}

void loop() {

  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();

  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}
