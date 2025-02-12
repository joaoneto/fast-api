# Build
CC = gcc
CFLAGS_DEBUG = -g -O0 -Wall -DAPPLOG_LEVEL=APPLOG_DEBUG
CFLAGS_RELEASE = -O2 -Wall
LDFLAGS = -luv

# Diretórios
SRC_DIR = src
INC_DIR = include
TARGET_DIR = target
DEBUG_DIR = $(TARGET_DIR)/debug
RELEASE_DIR = $(TARGET_DIR)/release

# Encontrar automaticamente todos os arquivos .c em src/ e subdiretórios
SRC_FILES = $(shell find $(SRC_DIR) -type f -name "*.c")

# Criar automaticamente a lista de objetos .o correspondente para Debug e Release
DEBUG_OBJS = $(patsubst $(SRC_DIR)/%.c, $(DEBUG_DIR)/%.o, $(SRC_FILES))
RELEASE_OBJS = $(patsubst $(SRC_DIR)/%.c, $(RELEASE_DIR)/%.o, $(SRC_FILES))

# Definir os binários de saída
DEBUG_BIN = $(DEBUG_DIR)/app
RELEASE_BIN = $(RELEASE_DIR)/app

all: debug

# Criar diretórios se não existirem
$(DEBUG_DIR) $(RELEASE_DIR):
	@mkdir -p $@

# Criar diretórios de objetos mantendo a estrutura de src/
$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.c | $(DEBUG_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_DEBUG) -I$(INC_DIR) -c $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.c | $(RELEASE_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS_RELEASE) -I$(INC_DIR) -c $< -o $@

# Linkar os objetos e gerar o binário final (modo Debug)
$(DEBUG_BIN): $(DEBUG_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Linkar os objetos e gerar o binário final (modo Release)
$(RELEASE_BIN): $(RELEASE_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Regras para Debug e Release
debug: $(DEBUG_BIN)
release: $(RELEASE_BIN)

# Limpar artefatos
clean:
	rm -rf $(TARGET_DIR)
