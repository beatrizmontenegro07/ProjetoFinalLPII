# Servidor de Chat Multiusuário (TCP) - Etapa 1
Aluna: Beatriz Montenegro Maia Chaves 

Matrícula: 20230012233

Este projeto tem como objetivo tem como objetivo desenvolver um servidor TCP concorrente que aceita a conexão de múltiplos clientes. Cada cliente será atendido por uma thread e as mensagens serão retrasmitidas para os demais clientes via broadcast. O diagrama abaixo mostra de forma visual o funcionamento da arquitetura do sistema: 

![alt text](docs/diagrama.png)


## Estrutura do repositório

O projeto está organizado nos seguintes diretórios:
- **bin/**: Contém os arquivos executáveis após a compilação
- **build/**: Contém os arquivos objeto (.o)
- **docs/**: Guarda a documentação do projeto (diagramas e relatórios)
- **include/**: Contém os arquivos de cabeçalho(.h) do projeto
- **src/**: Contém todo o código-fonte (.c), subdividido em:
    - **lib/**: Implementação de bibliotecas auxiliares
    - **server/**: Código-fonte principal do servidor (em andamento)
    - **cliente/**: Código-fonte principal do servidor (em andamento)
    - **tests/**: Códigos de teste para validar funcionalidades
- **Makefile**: Script para compilar e limpar o projeto


## Análise do Código

### Biblioteca de logging (libtslog.h)

O sistema de logging é um componente central e foi projetado para ser thread-safe. Isso é crucial, pois múltiplas threads (cada uma representando um cliente) precisarão escrever no mesmo arquivo de log simultaneamente.

- **Mecanismo de Sincronização**: A biblioteca utiliza `pthread_mutex_t` para garantir que toda a operação de escrita no arquivo seja feita de forma segura, assim apenas uma thread por vez tem direito de entrar na seção crítica e escrever a mensagem de log. 

- A biblioteca possui funções simples: `tslog_init()` para inicializar o log, `tslog_log()` para escrever a mensagem de log no arquivo e `tslog_destroy()` para finalizar o logger.


## Como Compilar e Executar

Para compilar e executar o projeto, você pode usar o compilador GCC diretamente ou utilizar o `Makefile` fornecido.

### Compilação com Makefile

1.  Certifique-se de que o arquivo `Makefile` se encontra-se no diretório atual.
2.  Abra o terminal e execute o comando:

    ```bash
    make
    ```
3.  Isso irá gerar um executável .


### Execução
Após a compilação, execute o teste do logger com o comando:
```bash
./bin/test_logger
```
O programa será executado e irá gerar o arquivo de log no diretório logs/test.log
