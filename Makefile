# Compilador
CC = gcc
# Flags de compilação: -g (debug), -Wall (warnings), -Iinclude (procurar headers em include/)
CFLAGS = -g -Wall -Iinclude
# Flags de linkagem: -lpthread é essencial para usar pthreads
LDFLAGS = -lpthread

# Diretórios
SRCDIR = src
INCDIR = include
BUILDDIR = build
BINDIR = bin

# Fontes da lib
LIB_SRC = $(SRCDIR)/lib/libtslog.c
LIB_OBJ = $(BUILDDIR)/libtslog.o

# Fontes do teste
TEST_SRC = $(SRCDIR)/tests/test_logger.c
TEST_OBJ = $(BUILDDIR)/test_logger.o
TEST_EXEC = $(BINDIR)/test_logger

# Regra principal (padrão)
all: $(TEST_EXEC)

# Regra para criar o executável de teste
$(TEST_EXEC): $(TEST_OBJ) $(LIB_OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Executável de teste criado em $(TEST_EXEC)"

# Regra para compilar o objeto de teste
$(BUILDDIR)/test_logger.o: $(TEST_SRC) $(INCDIR)/libtslog.h
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar o objeto da lib
$(BUILDDIR)/libtslog.o: $(LIB_SRC) $(INCDIR)/libtslog.h
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos compilados
clean:
	@echo "Limpando arquivos gerados..."
	rm -rf $(BUILDDIR)/* $(BINDIR)/* app.log

.PHONY: all clean