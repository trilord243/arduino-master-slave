#include <WiFi.h>
#include <WebServer.h>

#define RELAY_PIN 4

// Datos de red WiFi
const char* ssid = "cuarto_apa";
const char* password = "agfaer0201";

// IP fija para el Esclavo
IPAddress ip(192, 168, 1, 7);  
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Servidor web en el puerto 80
WebServer server(80);
bool reléCerrado = false;

// 🔹 Función para activar el relé
void activarRele() {
  Serial.println("📡 Señal recibida: Activando relé...");
  digitalWrite(RELAY_PIN, HIGH);
  delay(5000);
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("🔒 Relé desactivado.");
  
  reléCerrado = true;
  server.send(200, "text/plain", "Relé activado.");
}

// 🔹 Función para confirmar cierre
void confirmarCierre() {
  if (reléCerrado) {
    server.send(200, "text/plain", "cerrado");
    Serial.println("📡 Confirmación enviada al Maestro.");
    reléCerrado = false;
  } else {
    server.send(200, "text/plain", "esperando");
  }
}

// 🔹 Función para conectar a WiFi con IP fija
void conectarWiFi() {
  Serial.print("🌍 Conectando a WiFi...");
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\n✅ Conectado a WiFi");
  Serial.print("📡 Dirección IP fija: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  conectarWiFi();

  server.on("/activar", activarRele);
  server.on("/confirmar", confirmarCierre);
  server.begin();
  Serial.println("📡 Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}
