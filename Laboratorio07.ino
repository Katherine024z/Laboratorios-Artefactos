#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <LoRa.h>

// ======================================================
// IDENTIFICACIÓN
// ======================================================

const int MI_ID = 2;

// ======================================================
// OLED
// ======================================================

#define I2C_ADDRESS   0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ======================================================
// PINES LORA
// ======================================================

// definir los pines del módulo LoRa con #define
#define NSS 5
#define SCK 18
#define MOSI 23
#define MISO 19
// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("RECEPTOR LORA");

  Serial.print("Mi ID: ");
  Serial.println(MI_ID);

  // ----------------------------------------------------
  // OLED (mensaje de inicio)
  // ----------------------------------------------------

  Wire.begin();
  delay(250);

  if (!display.begin(I2C_ADDRESS, true)) {
    Serial.println("ERROR: OLED no encontrada");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("RECEPTOR LORA");

  display.setCursor(0, 18);
  display.print("ID: ");
  display.println(MI_ID);

  display.setCursor(0, 36);
  display.println("Esperando mensaje...");

  display.display();

  // ----------------------------------------------------
  // LORA
  // ----------------------------------------------------

  // iniciar el bus SPI con SPI.begin(...) usando los pines definidos
  // indicar a la librería los pines del módulo con LoRa.setPins(...)
  //       (NSS, RESET = -1, DIO0 = -1)
  SPI.begin(SCK,MISO,MOSI,NSS);

  LoRa.setPins(NSS,-1,-1);

  if (!LoRa.begin(915E6)) {

    Serial.println("ERROR: No se pudo iniciar LoRa");

    // OLED (mensaje de error)
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("ERROR LORA");
    display.display();

    while (true);
  }

  

  Serial.println("LoRa iniciado correctamente");
  Serial.println("Esperando mensajes...");
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  int packetSize = LoRa.parsePacket();

  if (packetSize) {

    String paquete = "";

    while (LoRa.available()) {
      paquete += (char)LoRa.read();
    }

    procesarMensaje(paquete);
  }
}

// ======================================================
// PROCESAR MENSAJE
// ======================================================

void procesarMensaje(String paquete) {

  Serial.println();
  Serial.print("Paquete recibido: ");
  Serial.println(paquete);

  // Formato: ORIGEN|DESTINO|MENSAJE
  int separador1 = paquete.indexOf('|');
  int separador2 = paquete.indexOf('|', separador1 + 1);

  if (separador1 == -1 || separador2 == -1) {
    Serial.println("Formato incorrecto");
    return;
  }

  int idOrigen = paquete.substring(0, separador1).toInt();
  int idDestino = paquete.substring(separador1 + 1, separador2).toInt();
  String mensaje = paquete.substring(separador2 + 1);

  Serial.print("Origen: ");
  Serial.println(idOrigen);

  Serial.print("Destino: ");
  Serial.println(idDestino);

  Serial.print("Mensaje cifrado: ");
  Serial.println(mensaje);

  // ----------------------------------------------------
  // COMPROBAR DESTINO
  // ----------------------------------------------------

  if (idDestino != MI_ID) {
    Serial.println("Mensaje ignorado.");
    return;
  }

  int rssi = LoRa.packetRssi();

  Serial.println("Mensaje aceptado");
  Serial.print("RSSI: ");
  Serial.println(rssi);

  // ----------------------------------------------------
  // OLED
  // ----------------------------------------------------

  // limpiar la pantalla y mostrar en la OLED:
  //       - el ID de origen ("Mensaje de ID:")
  //       - el mensaje cifrado recibido
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Mensaje de ID: ");
  display.println(idOrigen);

  //       - el texto "Mensaje cifrado:"
  display.setCursor(1, 5);
  display.print("Mensaje Cifrado: ");
  display.println(mensaje);

  //       - el RSSI
  display.setCursor(2, 5);
  display.print("RSSI: ");
  display.println(rssi);

  display.display(); 
}
