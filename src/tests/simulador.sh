#!/bin/bash

# --- Configurações ---
SERVER_IP="127.0.0.1"
NUM_CLIENTS=5

# --- Lógica do Script ---
# Navega para o diretório raiz do projeto para que os caminhos funcionem de forma confiável.
# O comando 'dirname "$0"' pega o diretório do script, e '/../../' sobe dois níveis.
cd "$(dirname "$0")/../../" || exit

LOG_DIR="logs" # Diretório para salvar os logs de cada cliente
CLIENT_EXEC="./bin/client" # Caminho para o executável do cliente

# --- Validação ---
# Verifica se o executável do cliente existe antes de continuar.
if [ ! -f "$CLIENT_EXEC" ]; then
    echo "Erro: O executável do cliente '$CLIENT_EXEC' não foi encontrado."
    echo "Por favor, compile o projeto com 'make' antes de rodar os testes."
    exit 1
fi

echo "Iniciando $NUM_CLIENTS clientes de teste contra o servidor em $SERVER_IP..."
echo "O output de cada cliente será salvo em arquivos no diretório '$LOG_DIR/'."

for i in $(seq 1 $NUM_CLIENTS); do
    # O pipe (|) envia a entrada de texto para o stdin do processo cliente.
    # A saída do cliente (tanto normal quanto de erro) é redirecionada para um arquivo de log.
    # O '&' no final executa cada cliente em background, permitindo que todos rodem simultaneamente.
    (
        echo "Tester_${i}"
        sleep 1
        echo "Olá, esta é a mensagem do Tester ${i}."
        sleep 1
        echo "0" # Envia "0" para que o cliente se desconecte graciosamente.
    ) | "$CLIENT_EXEC" "$SERVER_IP" > "${LOG_DIR}/client_${i}.log" 2>&1 &
done

echo "Aguardando todos os clientes finalizarem..."
wait # Espera que todos os processos em background terminem.

echo "Teste dos clientes concluído."
echo "Verifique os arquivos em '$LOG_DIR' para ver o output de cada cliente."