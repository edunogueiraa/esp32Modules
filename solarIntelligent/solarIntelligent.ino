#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "configs.h"

// Configurações do Display OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C 

// Inicializa o objeto do display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long ultimoEnvio = 0;
float tensao   = 0.0;
float corrente = 0.0;
float potencia = 0.0;

float lerTensao() {
  int leitura = analogRead(VT_PIN); // 14
  float tensao = (leitura * (ESP_VOLTAGEM / 1023.0)) * 5;
  return tensao;
}

float lerCorrente() {
  int leitura = analogRead(AT_PIN);
  float corrente = (leitura * (ESP_VOLTAGEM / 1023.0));
  return corrente;
}

float calcularPotencia(float tensao, float corrente){
  potencia = corrente * tensao;
  return potencia;  
}

void atualizarDisplay(float tensao, float corrente, float potencia) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 5); 
  display.print("Monitor Solar ");
  display.setCursor(0, 20);
  display.print("V: ");
  display.print(tensao, 2); 
  display.setCursor(0, 35);
  display.print("A: ");
  display.print(corrente, 2); 
  display.setCursor(0, 50);
  display.print("W: ");
  display.print(potencia, 2); 
  display.display();
}

String obterTimestamp() {
  struct tm info;
  if (!getLocalTime(&info, 200)) return "";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &info);
  return String(buf);
}

void conectarWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

void verificarConexao(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado!");
    digitalWrite(25,LOW);
  } else {
    digitalWrite(25,HIGH);
  }
}

void dadosSerial(float tensao, float corrente, float potencia){
  Serial.print("Tensao: ");
  Serial.print(tensao, 2);
  Serial.println("V");
  Serial.print("Corrente: ");
  Serial.print(corrente, 2);
  Serial.println("A");
  Serial.print("Potencia: ");
  Serial.print(potencia, 2);
  Serial.println("W");
  Serial.println();
}

String geradorJson(float tensao,float corrente,float potencia){
  String ts = obterTimestamp();
  String json = "{";
    json += "\"station_id\":\"" + String(STATION_ID) + "\",";
    if (ts.length() > 0) json += "\"timestamp\":\"" + ts + "\",";
  
    json += "\"location\":{";
    json +=   "\"type\":\"Point\",";
    json +=   "\"coordinates\":[-0.000000,-0.000000]"; 
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
    json +=   "\"temperature_c\":0.00,";      
    json +=   "\"humidity_percent\":0.00,";   
    json +=   "\"irradiance_w_m2\":0.00";     
    json += "},";
  
    json += "\"network\":{";
    json +=   "\"interface\":\"WiFi\",";
    json +=   "\"ssid\":\"" + String(WIFI_SSID) + "\",";
    json +=   "\"signal_dbm\":" + String(WiFi.RSSI()) + ",";
    json +=   "\"ip\":\"" + WiFi.localIP().toString() + "\"";
    json += "},";
  
    json += "\"device\":{";
    json +=   "\"firmware\":\"0.0.0\",";
    json +=   "\"hardware_version\":\"1.0\",";
    json +=   "\"uptime_s\":" + String(millis() / 1000);
    json += "}";

  json += "}";

  return json;
}

void enviarDados(String json){
  WiFiClient cliente;
  HTTPClient http;

  if (!http.begin(cliente, SERVER_URL)) {
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
}

void setup() {
  Serial.begin(115200);
  pinMode(25, OUTPUT);
  analogReadResolution(10);   
  analogSetAttenuation(ADC_11db); 
  Wire.begin(21, 22);// (SDA=21, SCL=22)
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Falha ao inicializar SSD1306");
    for(;;);
  }
  display.clearDisplay();
  display.display();
  conectarWiFi();
  configTime(0, 0, "pool.ntp.org", "time.google.com"); //servidores públicos de NTP
}

void loop() {
  verificarConexao();
  tensao   = lerTensao();
  corrente = lerCorrente();
  potencia = calcularPotencia(tensao, corrente);
  atualizarDisplay(tensao, corrente, potencia);
  dadosSerial(tensao, corrente, potencia);

  if (millis() - ultimoEnvio >= 60000UL) {
    ultimoEnvio = millis();
    String json = geradorJson(tensao, corrente, potencia);
    enviarDados(json);
  }
  delay(500);
}
