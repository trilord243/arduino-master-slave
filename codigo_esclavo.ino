#include <esp_now.h>
#include <WiFi.h>

#define LED_PIN 4 ///< Pin donde está conectado el LED

bool estadoBoton = false; ///< Estado actual del botón

typedef struct struct_message {
  bool estadoBoton;
} struct_message;

struct_message datosRecibidos; ///< Variable para almacenar los datos recibidos

// Callback para recibir datos de ESP-NOW
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  Serial.println("📡 Datos recibidos!");

  // Mostrar la MAC del remitente (opcional, para depuración)
  Serial.print("🔎 MAC del remitente: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  // Procesar los datos recibidos
  memcpy(&datosRecibidos, data, sizeof(datosRecibidos));
  estadoBoton = datosRecibidos.estadoBoton;

  Serial.print("🔄 Estado recibido: ");
  Serial.println(estadoBoton);

  if (estadoBoton) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("💡 LED ENCENDIDO");
  } else {
    digitalWrite(LED_PIN, LOW);
    Serial.println("💡 LED APAGADO");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error al iniciar ESP-NOW. Reiniciando...");
    delay(1000);
    ESP.restart();
  }

  esp_now_register_recv_cb(OnDataRecv); // Registra la función para recibir datos

  Serial.println("📡 Esclavo listo para recibir datos...");
}

void loop() {
}
