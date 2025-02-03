#include <ESP32QRCodeReader.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Configuración del lector QR, Bluetooth y WiFi
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
BluetoothSerial SerialBT;

const char *ssid = "arduino-test";
const char *password = "123456";

String serverURL = "http://192.168.4.1/data"; // Ruta del servidor del segundo Arduino
bool wifiConnected = false;

// Función para conectar a WiFi
void connectToWiFi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConexión WiFi establecida.");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    Serial.println("\nNo se pudo conectar al WiFi.");
    wifiConnected = false;
  }
}

// Función para enviar datos al servidor web
void sendToWebServer(const String &data) {
  if (!wifiConnected) {
    connectToWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "data=" + data;
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0) {
      Serial.print("Respuesta del servidor: ");
      Serial.println(http.getString());
    } else {
      Serial.print("Error en la solicitud HTTP: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("No se pudo conectar al servidor. Verifique la conexión WiFi.");
  }
}

// Función para probar la conexión con el servidor
void sendTestToServer() {
  if (!wifiConnected) {
    connectToWiFi();
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://192.168.4.1/test");
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("Respuesta del servidor: ");
      Serial.println(http.getString());
    } else {
      Serial.print("Error en la solicitud HTTP: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("No se pudo conectar al servidor. Verifique la conexión WiFi.");
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializar Bluetooth
  if (!SerialBT.begin("ESP32_QR_Reader")) {
    Serial.println("Error al iniciar Bluetooth");
    while (true);
  }
  Serial.println("Bluetooth iniciado, esperando conexión...");

  // Inicializar lector QR
  reader.setup();
  reader.beginOnCore(1);

  Serial.println("Esperando códigos QR...");
}

void loop() {
  struct QRCodeData qrCodeData;

  // Leer códigos QR
  if (reader.receiveQrCode(&qrCodeData, 100)) {
    if (qrCodeData.valid) {
      String payload = (const char *)qrCodeData.payload;
      Serial.println("Código QR válido encontrado:");
      Serial.println(payload);

      // Enviar datos por Bluetooth
      if (SerialBT.hasClient()) {
        SerialBT.println(payload);
      } else {
        Serial.println("No hay clientes Bluetooth conectados.");
      }
    } else {
      Serial.println("Código QR inválido.");
    }
  }

  // Leer datos desde el Bluetooth
  if (SerialBT.available()) {
    String received = SerialBT.readStringUntil('\n');
    Serial.println("Mensaje recibido por Bluetooth: " + received);

    if (received == "habilitado") {
      Serial.println("Se recibió la señal: habilitado");

      // Enviar datos al servidor web
      sendToWebServer("habilitado");
    }
  }

  delay(100);
}
