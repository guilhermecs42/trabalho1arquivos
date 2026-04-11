# Cleyton José Rodrigues Macedo 16821725
# Guilherme Cavalcanti de Santana 15456556

# Variáveis
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinclude -g
# ADICIONE A SEGUINTE FLAG NA LINHA ACIMA PARA COMPILAR COM DEBUG: -DPRINT_ERROS
TARGET  := CPTM

# Pastas
SRC_DIR := src
OBJ_DIR := obj

# Arquivos
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Regras
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run:
	./$(TARGET) 

.PHONY: all clean
