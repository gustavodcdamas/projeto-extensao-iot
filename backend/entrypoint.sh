#!/bin/bash
set -e

echo "Aguardando o banco de dados em $DB_HOST:$DB_PORT..."
python3 -c "
import socket, time, os
host = os.getenv('DB_HOST', '10.8.0.3')
port = int(os.getenv('DB_PORT', 5432))
while True:
    try:
        with socket.create_connection((host, port), timeout=2):
            break
    except OSError:
        time.sleep(1)
"
echo "Banco online! Aplicando migrações de schema..."
PGPASSWORD=$DB_PASSWORD psql -h $DB_HOST -p $DB_PORT -U $DB_USER -d $DB_NAME -f /app/database/schema.sql

echo "Iniciando Consumer MQTT..."
exec python3 /app/src/consumer.py