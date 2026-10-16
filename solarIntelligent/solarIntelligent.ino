#include <WiFi.h>
#include <HTTPClient.h>
#include <U8glib.h>
// #include "configs.h"

const char* ssid     = "XXXXX";
const char* password = "XXXXX";

const char* serverName = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
const char* serverUrl  = "http://XXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

#define CLIENT "NAME"
#define TYPE   "NAME"


#define VT_PIN 34
#define AT_PIN 35


#define ESP_VOLTAGEM 3.3

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

float lerTensao() {
  int leitura = analogRead(VT_PIN);
  float tensao = leitura * (ESP_VOLTAGEM / 1023.0) * 5;
  return tensao;
}

float lerCorrente() {
  int leitura = analogRead(AT_PIN);
  float corrente = leitura * (ESP_VOLTAGEM / 1023.0);
  return corrente;
}

void setup() {

  u8g.setFont(u8g_font_unifont);
  Serial.begin(921600);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

}

void loop() {

  float corrente = lerCorrente();
  float tensao   = lerTensao();
  float potencia = corrente * tensao;

  Serial.print("Tensao: ");
  Serial.print(tensao, 2);
  Serial.println("V");

  Serial.print("Corrente: ");
  Serial.print(corrente, 2);
  Serial.println("mA");

  Serial.print("Potencia: ");
  Serial.print(potencia, 2);
  Serial.println("W");

  u8g.firstPage();

  do {
    u8g.setPrintPos(0, 16);
    u8g.print("Monitor Solar");

    u8g.setPrintPos(0, 30);
    u8g.print("W: ");
    u8g.print(potencia, 3);

    u8g.setPrintPos(0, 45);
    u8g.print("V: ");
    u8g.print(tensao, 3);

    u8g.setPrintPos(0, 60);
    u8g.print("mA: ");
    u8g.print(corrente * 1000.0, 1);  // convertido de A para mA

  } while (u8g.nextPage());

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado!");
    return;
  }

  String json = "{";

  json += "\"station_id\":\"EBIKE-ST-001\",";
  json += "\"timestamp\":\"2026-10-02T14:30:00Z\",";

  json += "\"location\":{";
  json +=   "\"type\":\"Point\",";
  json +=   "\"coordinates\":[-34.845000,-7.119500]";
  json += "},";

  json += "\"status\":{";
  json +=   "\"online\":true,";
  json +=   "\"operational\":true,";
  json +=   "\"available_slots\":2,";
  json +=   "\"total_slots\":4";
  json += "},";

  json += "\"generation\":{";
  json +=   "\"source\":\"solar\",";
  json +=   "\"installed_power_w\":1200,";
  json +=   "\"current_power_w\":" + String(potencia, 2) + ",";
  json +=   "\"energy_generated_wh\":5832.70";
  json += "},";

  json += "\"energy\":{";
  json +=   "\"voltage_v\":" + String(tensao, 2) + ",";
  json +=   "\"current_a\":" + String(corrente, 3) + ",";
  json +=   "\"power_w\":"   + String(potencia, 2) + ",";
  json +=   "\"battery_soc_percent\":76.50";
  json += "},";

  json += "\"charging\":{";
  json +=   "\"active\":true,";
  json +=   "\"active_slots\":2,";
  json +=   "\"charging_power_w\":" + String(potencia * 0.7, 2) + ",";
  json +=   "\"energy_delivered_wh\":3842.60,";
  json +=   "\"sessions_today\":17";
  json += "},";

  json += "\"environment\":{";
  json +=   "\"temperature_c\":31.40,";
  json +=   "\"humidity_percent\":68.20,";
  json +=   "\"irradiance_w_m2\":745.80";
  json += "},";

  json += "\"network\":{";
  json +=   "\"interface\":\"WiFi\",";
  json +=   "\"ssid\":\"" + String(ssid) + "\",";
  json +=   "\"signal_dbm\":" + String(WiFi.RSSI()) + ",";
  json +=   "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += "},";

  json += "\"device\":{";
  json +=   "\"firmware\":\"1.4.2\",";
  json +=   "\"hardware_version\":\"1.0\",";
  json +=   "\"uptime_s\":" + String(millis() / 1000);
  json += "}";

  json += "}";

  Serial.println(json);


  WiFiClient cliente;
  HTTPClient http;

  if (!http.begin(cliente, serverUrl)) {
    Serial.println("Não foi possível iniciar HTTP.");
    return;
  }

  http.addHeader("Content-Type", "application/json");

  Serial.println("\nEnviando dados...");
  Serial.println(json);

  int codigoHTTP = http.POST(json);

  if (codigoHTTP > 0) {
    Serial.printf("Resposta HTTP: %d\n", codigoHTTP);
    Serial.println(http.getString());
  } else {
    Serial.print("Erro HTTP: ");
    Serial.println(http.errorToString(codigoHTTP));
  }

  http.end();

  delay(60000);
}
