#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32QRCodeReader.h>

#define BUTTON_PIN 2

// Datos de red WiFi
const char* ssid = "cuarto_apa";  
const char* password = "agfaer0201";  

// IP del Esclavo en la misma red
const char* esclavoIP = "192.168.1.7";
const int esclavoPuerto = 80;

ESP32QRCodeReader qrCodeReader(CAMERA_MODEL_AI_THINKER);
QRCodeData qrCodeData;

String serverUrl = "https://visitec-backend-production.up.railway.app/api/visits/validate/";
String macAddress = "";

// Estado de espera de confirmación
bool esperandoConfirmacion = false;

// 🔹 Función para conectar a WiFi
void conectarWiFi() {
  Serial.print("🌍 Conectando a WiFi...");
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 15) {
    delay(1000);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Conectado a WiFi");
    Serial.print("📡 Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ No se pudo conectar a WiFi.");
  }
}

// 🔹 Función para validar QR con el backend
bool validarQRCode(String idQR) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi no conectado.");
    return false;
  }

  HTTPClient http;
  String url = serverUrl + idQR + "/accessControl";
  Serial.println("🌍 Conectando a: " + url);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{\"macAddress\": \"00:1A:3F:F1:4C:C6\"}";
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("🔎 Respuesta del servidor: " + response);
    http.end();

    if (response.indexOf("\"allowed\":true") != -1) {
      Serial.println("✅ Acceso permitido.");
      return true;
    } else {
      Serial.println("🚫 Acceso denegado.");
      return false;
    }
  } else {
    Serial.print("❌ Error en la solicitud HTTP. Código: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

// 🔹 Función para enviar señal al esclavo
bool enviarSenalAlEsclavo() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ No conectado a WiFi, no se puede enviar señal.");
    return false;
  }

  HTTPClient http;
  String url = "http://" + String(esclavoIP) + ":" + String(esclavoPuerto) + "/activar";
  Serial.println("📡 Enviando señal al esclavo: " + url);

  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("✅ Respuesta del esclavo: ");
    Serial.println(http.getString());
    http.end();
    return true;
  } else {
    Serial.print("❌ Error al enviar la señal. Código: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

// 🔹 Función para recibir confirmación del esclavo
bool esperarConfirmacionEsclavo() {
  HTTPClient http;
  String url = "http://" + String(esclavoIP) + ":" + String(esclavoPuerto) + "/confirmar";
  Serial.println("📡 Esperando confirmación del esclavo...");

  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String respuesta = http.getString();
    Serial.print("🔎 Respuesta del esclavo: ");
    Serial.println(respuesta);
    http.end();

    return (respuesta == "cerrado");
  } else {
    Serial.print("❌ Error al recibir confirmación. Código: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
}

// 🔹 Función para leer QR y activar el esclavo
void leerQRCode() {
  if (esperandoConfirmacion) {
    Serial.println("⏳ Esperando confirmación del esclavo. No se aceptan nuevos QR.");
    return;
  }

  if (qrCodeReader.receiveQrCode(&qrCodeData, 100)) {
    String idQR = (const char *)qrCodeData.payload;
    idQR.trim();

    Serial.print("📷 QR Code recibido: ");
    Serial.println(idQR);

    if (validarQRCode(idQR)) {
      Serial.println("📡 Enviando señal al esclavo...");
      if (enviarSenalAlEsclavo()) {
        esperandoConfirmacion = true;

        // Esperar confirmación del esclavo
        while (!esperarConfirmacionEsclavo()) {
          Serial.println("⏳ Aguardando confirmación...");
          delay(2000);
        }

        Serial.println("✅ Confirmación recibida. Se aceptan nuevos QR.");
        esperandoConfirmacion = false;
      }
    } else {
      Serial.println("❌ No autorizado.");
    }
  }
}

void setup() {
  Serial.begin(115200);
  conectarWiFi();

  qrCodeReader.setup();
  qrCodeReader.beginOnCore(1);
  Serial.println("📷 Esperando códigos QR...");
}

void loop() {
  leerQRCode();
  delay(2000);
}
