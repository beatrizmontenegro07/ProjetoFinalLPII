MAKEFLAGS += --no-print-directory

# Compilador C
CC = gcc

# --- Estrutura de Diretórios (Caminhos Relativos) ---
BIN_DIR     := bin
BUILD_DIR   := build
SRC_DIR     := src

# Flags de compilação:
# -Wall: Todos os avisos
# -g: Informações de debug
# -Iinclude: Usa o caminho relativo para a pasta de headers
CFLAGS = -Wall -g -Iinclude

# Flags do linker: -pthread (para a biblioteca de threads)
LDFLAGS = -pthread


# --- VPATH: Caminho de Busca para Arquivos Fonte ---
VPATH = $(SRC_DIR)/lib:$(SRC_DIR)/server:$(SRC_DIR)/cliente:$(SRC_DIR)/tests

# --- Definição dos Arquivos Objeto ---
SERVER_OBJS      := $(addprefix $(BUILD_DIR)/, server.o main_server.o libtslog.o)
CLIENT_OBJS      := $(addprefix $(BUILD_DIR)/, cliente.o main_cliente.o libtslog.o)

# --- Definição dos Alvos (Executáveis) ---
TARGET_SERVER    := $(BIN_DIR)/server
TARGET_CLIENT    := $(BIN_DIR)/client

.PHONY: all clean test

# --- Regras Principais ---
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# --- Regras de Linkagem ---
$(TARGET_SERVER): $(SERVER_OBJS)
	@echo "Linkando o executável do Servidor..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) $(SERVER_OBJS) -o $@

$(TARGET_CLIENT): $(CLIENT_OBJS)
	@echo "Linkando o executável do Cliente..."
	@mkdir -p $(BIN_DIR)
	$(CC) $(LDFLAGS) $(CLIENT_OBJS) -o $@


# --- Regra de Compilação Única ---
$(BUILD_DIR)/%.o: %.c
	@echo "Compilando $<..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


# --- Regra de Limpeza ---
clean:
	@echo "Limpando diretórios bin/ e build/..."
	@rm -rf $(BIN_DIR) $(BUILD_DIR)

# --- Regra de Teste com Script ---
# Garante que o cliente está compilado e depois executa o script de teste.
test: $(TARGET_CLIENT)
	@echo "Executando script de teste em BASH..."
	@chmod +x $(SRC_DIR)/tests/simulador.sh
	@$(SRC_DIR)/tests/simulador.sh