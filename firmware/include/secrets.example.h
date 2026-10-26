#ifndef SECRETS_H
#define SECRETS_H

// Configurações de Rede Wi-Fi
#define WIFI_SSID "SEU_WIFI_AQUI"
#define WIFI_PASSWORD "SUA_SENHA_WIFI_AQUI"

// Configurações do Broker MQTT (Criptografado / MQTTS)
#define MQTT_SERVER "mqtt.gustavodcdamas.com.br"
#define MQTT_PORT_SECURE 8883 // Porta segura padrão TLS

// Credenciais de Autenticação no Broker (Evita conexões anônimas)
#define MQTT_USER "estacao_m5stick"
#define MQTT_PASSWORD "SENHA_FORTE_DO_BROKER_AQUI"

#endif