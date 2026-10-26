# Estação de Monitoramento Ambiental e Alagamentos (IoT Segura)

Projeto de extensão universitária voltado para cidades inteligentes (*Smart Cities*), utilizando **ESP32 (M5StickC Plus 2)**, múltiplos sensores ambientais e uma arquitetura em nuvem segura com criptografia de ponta a ponta.

## 🛠️ Arquitetura e Tecnologias
- **Edge / Dispositivo:** M5StickC Plus 2 + DHT22 (Temperatura/Umidade) + HC-SR04 (Nível de Água / Alagamento).
- **Protocolo de Comunicação:** MQTT sobre TLS (**MQTTS**, porta 8883) com autenticação por credenciais.
- **Camada de Rede / DNS:** Cloudflare com proxy reverso e gerenciamento de certificados SSL/TLS.
- **Backend / Consumer:** Python (Paho-MQTT + Psycopg2) empacotado em container Docker.
- **Banco de Dados:** PostgreSQL com persistência e criação automatizada via migrations.
- **CI/CD:** GitHub Actions para automação de build e publicação de imagens de container.

## 🔒 Segurança Implementada
1. **Tráfego Criptografado:** O ESP32 utiliza `WiFiClientSecure` para enviar pacotes via MQTTS, impedindo ataques de *sniffing* na rede Wi-Fi.
2. **Autenticação no Broker:** O Mosquitto exige usuário e senha definidos em arquivos de segredo locais.
3. **Proteção de Credenciais:** Arquivos `secrets.h` e `.env` são ignorados pelo Git (`.gitignore`), impedindo vazamento de senhas em repositórios públicos.

## 🚀 Como Executar o Projeto

### 1. Configurando o Firmware (M5StickC)
1. Abra a pasta `firmware/` no VS Code com a extensão PlatformIO.
2. Copie o arquivo de exemplo de segredos:
   ```bash
   cp firmware/include/secrets.example.h firmware/include/secrets.h
3. Edite o secrets.h preenchendo seu Wi-Fi e suas credenciais MQTT.
4. Conecte o M5StickC Plus 2 via USB e faça o upload (pio run --target upload).

2. Subindo o Backend na Nuvem

1. No seu servidor, crie o arquivo .env com as credenciais do banco e broker:
    ```Snippet de código

    DB_HOST=10.8.0.3
    DB_PORT=5432
    DB_NAME=iot_db
    DB_USER=postgres
    DB_PASSWORD=sua_senha
    MQTT_BROKER=mqtt.gustavodcdamas.com.br
    MQTT_PORT=1883

2. Suba a aplicação usando Docker Compose:
    ```Bash

    docker-compose up -d