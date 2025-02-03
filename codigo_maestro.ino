#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ESP32QRCodeReader.h>

#define BUTTON_PIN 2

// MAC del esclavo (receptor)
uint8_t broadcastAddress[] = {0x08, 0xD1, 0xF9, 0x33, 0xEC, 0x98};

typedef struct struct_message {
  bool estadoBoton;
} struct_message;

struct_message DatoParaEnviar;
esp_now_peer_info_t peerInfo;

ESP32QRCodeReader qrCodeReader(CAMERA_MODEL_AI_THINKER);
String validador = "2424aa55";
QRCodeData qrCodeData;

bool estadoActualBoton = false; ///< Estado actual del botón

void leerQRCode() {
  if (qrCodeReader.receiveQrCode(&qrCodeData, 100)) {
    
    String payload = (const char *)qrCodeData.payload;

    // Eliminar espacios y caracteres invisibles antes de la comparación
    String cleanPayload = payload;
    cleanPayload.trim();

    Serial.print("📷 QR Code recibido: [");
    Serial.print(cleanPayload);
    Serial.print("] (Longitud: ");
    Serial.print(cleanPayload.length());
    Serial.println(")");

    if (cleanPayload == validador) {
      
      Serial.println("✅ QR Code validado correctamente.");

      // Alternar estado del botón
      estadoActualBoton = !estadoActualBoton;
      DatoParaEnviar.estadoBoton = estadoActualBoton;

      Serial.println("📡 Intentando enviar datos al esclavo...");
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DatoParaEnviar, sizeof(DatoParaEnviar));

      if (result == ESP_OK) {
        Serial.println("✅ Enviado con éxito");
      } else {
        Serial.println("❌ Error al enviar la información");
      }
    } else {
      Serial.println("❌ QR Code no válido.");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error al iniciar ESP-NOW, reiniciando...");
    delay(1000);
    ESP.restart();
  }

  // Configuración del peer (esclavo)
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Error al añadir el esclavo");
    return;
  }

  // Inicializar lector QR
  qrCodeReader.setup();
  qrCodeReader.beginOnCore(1);

  Serial.println("📷 Esperando códigos QR...");
}

void loop() {
  leerQRCode();
  delay(2000);
}
