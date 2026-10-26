import os
import json
import paho.mqtt.client as mqtt
import psycopg2

DB_HOST = os.getenv("DB_HOST", "10.8.0.3")
DB_PORT = os.getenv("DB_PORT", "5432")
DB_NAME = os.getenv("DB_NAME", "iot_db")
DB_USER = os.getenv("DB_USER", "postgres")
DB_PASSWORD = os.getenv("DB_PASSWORD", "")

MQTT_BROKER = os.getenv("MQTT_BROKER", "mqtt.gustavodcdamas.com.br")
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883)) # Porta interna do container para o mosquitto
MQTT_TOPIC = "extensao/iot/estacao"

def get_db_connection():
    return psycopg2.connect(
        host=DB_HOST,
        port=DB_PORT,
        database=DB_NAME,
        user=DB_USER,
        password=DB_PASSWORD
    )

def on_connect(client, userdata, flags, rc):
    print(f"Consumer conectado ao Broker MQTT com código {rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload_str = msg.payload.decode('utf-8')
        data = json.loads(payload_str)
        print(f"Dado seguro recebido: {data}")

        conn = get_db_connection()
        cur = conn.cursor()
        
        cur.execute(
            """
            INSERT INTO sensor_readings (device_id, temperature, humidity, water_level_cm)
            VALUES (%s, %s, %s, %s)
            """,
            (
                data.get("device"),
                data.get("temperature"),
                data.get("humidity"),
                data.get("water_level_cm")
            )
        )
        conn.commit()
        cur.close()
        conn.close()
        print("Dados gravados com segurança no PostgreSQL!")
        
    except Exception as e:
        print(f"Erro ao processar mensagem MQTT/Banco: {e}")

if __name__ == "__main__":
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    print(f"Iniciando consumer em {MQTT_BROKER}:{MQTT_PORT}...")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_forever()