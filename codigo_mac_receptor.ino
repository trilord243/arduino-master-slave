#include "WiFi.h"

void setup() {
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin();
    delay(100);  // Dar tiempo para la inicialización

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
}

void loop() {
    // No es necesario hacer nada en el loop
}
