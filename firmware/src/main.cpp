#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "secrets.h"

// Configurações do Broker MQTT Seguro
const char* mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT_SECURE;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;
const char* mqtt_topic = "extensao/iot/estacao";

// Configuração do Sensor DHT22 no TOPO (G26)
#define DHTPIN 26     
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// Configuração do Sensor HC-SR04 (Alagamento / Nível de Água)
#define TRIG_PIN 32
#define ECHO_PIN 33
#define SOUND_SPEED 0.034

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long ultimoEnvioMQTT = 0;
unsigned long ultimaAtualizacaoTela = 0;

// Variáveis globais para armazenar as leituras atuais dos sensores
float temperatura = 0.0;
float umidade = 0.0;
float distancia = 0.0;

// Função para desenhar o painel completo na tela (Incluindo IP e MAC)
void atualizarTela(bool wifiStatus, bool mqttStatus) {
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(1);
    
    M5.Display.println("--- ESTACAO IoT ---");
    M5.Display.printf("Temp: %.1f C | Umid: %.1f%%\n", temperatura, umidade);
    M5.Display.printf("Dist: %.1f cm\n", distancia);
    M5.Display.println("-------------------");
    
    // Status de Rede com IP e MAC
    if (wifiStatus) {
        M5.Display.println("Wi-Fi: [CONECTADO]");
        M5.Display.println("IP: " + WiFi.localIP().toString());
        M5.Display.println("MAC: " + WiFi.macAddress());
    } else {
        M5.Display.println("Wi-Fi: [DESCONECTADO]");
    }
    
    // Status do MQTT
    M5.Display.print("MQTT: ");
    M5.Display.println(mqttStatus ? "[ONLINE]" : "[OFFLINE/AGUARDANDO]");
}

void setup_wifi() {
    delay(10);
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(1);
    M5.Display.print("Conectando Wi-Fi:\n" + String(WIFI_SSID));

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        M5.Display.print(".");
        tentativas++;
        
        if (tentativas > 40) {
            M5.Display.fillScreen(BLACK);
            M5.Display.setCursor(0, 0);
            M5.Display.println("Erro: Falha Wi-Fi!\nReiniciando...");
            delay(3000);
            ESP.restart();
        }
    }

    Serial.println("\nWiFi conectado! IP: " + WiFi.localIP().toString());
    Serial.println("MAC Address: " + WiFi.macAddress());
}

void tentarConectarMQTT() {
    if (!client.connected() && WiFi.status() == WL_CONNECTED) {
        String clientId = "M5StickC-Estacao-";
        clientId += String(random(0xffff), HEX);
        
        // Passando usuário e senha configurados no secrets.h
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
            Serial.println("Conectado ao Broker MQTT na Nuvem com sucesso!");
        } else {
            // ISSO VAI MOSTRAR O MOTIVO EXATO DA FALHA NO MONITOR SERIAL:
            Serial.print("Falha na conexao MQTT. Estado (rc) = ");
            Serial.println(client.state());
            // Significado dos códigos rc comuns do PubSubClient:
            // -4 : MQTT_CONNECTION_TIMEOUT (Servidor não respondeu / Porta bloqueada)
            // -2 : MQTT_CONNECT_FAILED (Falha de rede)
            //  5 : MQTT_NOT_AUTHORIZED (Usuário ou senha inválidos)
        }
    }
}

float medirDistancia() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration == 0) return -1.0;
    
    return duration * SOUND_SPEED / 2;
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.setTextColor(WHITE);

    Serial.begin(115200);
    dht.begin();
    
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    setup_wifi();

    espClient.setInsecure();
    client.setServer(mqtt_server, mqtt_port);
}

void loop() {
    M5.update();

    unsigned long agora = millis();

    // 1. Leitura contínua dos sensores a cada 2 segundos
    if (agora - ultimaAtualizacaoTela > 2000) {
        ultimaAtualizacaoTela = agora;

        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float dist = medirDistancia();

        if (!isnan(h) && !isnan(t)) {
            temperatura = t;
            umidade = h;
        }
        if (dist >= 0) {
            distancia = dist;
        }

        bool wifiOk = (WiFi.status() == WL_CONNECTED);
        bool mqttOk = client.connected();
        atualizarTela(wifiOk, mqttOk);
    }

    // 2. Tenta manter o MQTT conectado e envia os dados a cada 10 segundos
    if (!client.connected()) {
        tentarConectarMQTT();
    }
    client.loop();

    if (agora - ultimoEnvioMQTT > 10000) {
        ultimoEnvioMQTT = agora;

        if (client.connected()) {
            String payload = "{\"device\": \"m5stick-estacao-01\", "
                             "\"temperature\": " + String(temperatura, 1) + ", "
                             "\"humidity\": " + String(umidade, 1) + ", "
                             "\"water_level_cm\": " + String(distancia, 1) + "}";
            
            bool enviado = client.publish(mqtt_topic, payload.c_str());
            if (enviado) {
                Serial.println("Publicado com sucesso: " + payload);
            }
        } else {
            Serial.println("MQTT Offline. Coletando dados localmente...");
        }
    }
}