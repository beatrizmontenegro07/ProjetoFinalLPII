# Documentação da Interação com a IA (Prompts)

Este projeto foi desenvolvido com o auxílio de LLMs. Abaixo está os prompts utilizados.

## Prompt 1
> "Me ajude a criar a estrutura inicial da biblioteca de logger (libtslog.c). Preciso que o cabeçalho (libtslog.h) possua funções para inicializar, logar mensagens com níveis e destruir o logger. A função de log deve ser thread-safe, usando pthread_mutex_t para proteger o acesso ao arquivo.

 - **Objetivo**: Desenvolver uma biblioteca de logging segura, garantindo que as mensagens de diferentes threads não corrompam o arquivo de log.

 - **Contribuição**: A resposta gerou a estrutura da libtslog, com o cabeçalho tslog.h com os protótipos das funções e a implementação inicial da tslog.c com o uso de pthread_mutex para garantir a exclusão mútua.
"
### Prompt 2
> "Crie um Makefile  que deve compilar os código-fonte que estão em src/lib/tslog.c e src/tests/test_logger.c, usando os headers de include/. Os arquivos objeto (.o) devem ir para build/ e o executável final (test_logger) para bin/"

 - **Objetivo**: Simplificar o processo de compilação do código-fonte.

 - **Contribuição**: Gerou um Makefile que automatiza a compilação para testar o funcionamento do logger.

### Prompt 3
> "Tendo em vista o objetivo do projeto, me ajude a criar a arquitetura inicial do projeto definindo as headers principais para o servidor e o cliente"

 - **Objetivo:**  Estruturar a arquitetura do servidor de chat, definindo as estruturas de dados e os protótipos de funções principais.

 - **Contribuição**: Forneceu o código inicial para as headers princiapais, criando o `common.h` para estruturas compartilhadas entre o cliente e o servidor, `server.h` para a definição das estruturas e protótipos das funções do servidor e `client.h` para as do cliente.

### Prompt 4
> "Com base nas headers server.h e cliente.h, me ajude a criar e a entender as funções princiapais."

- **Objetivo:**  Desenvolver o código inicial para o servidor e o cliente, implementando a lógica de concorrência e comunicação.

- **Contribuição:** Forneceu uma visão clara da arquitetura multithread, detalhando o ciclo de vida de cada conexão no servidor e a separação das threads no cliente. Além disso, gerou a parte inicial do server.c e cliente.c.

### Prompt 5
> "Meu programa cliente não encerra completamente quando eu digito '0' para sair. Ele mostra a menssagem 'Desconectando...',  mas o terminal trava. O que está causando esse deadlock?"

- **Objetivo:**   Identificar e corrigir um bug crítico que impedia o encerramento correto do programa cliente.

- **Contribuição:** Ajudou a diagnosticar um deadlock entre a thread principal e a thread de recebimento. A solução proposta foi usar shutdown(socket_fd, SHUT_RDWR) para desbloquear a chamada recv, permitindo um encerramento limpo.

### Prompt 6
> "Me ajude a criar um script de teste (simulador.sh) para simular múltiplos clientes. O script deve se conectar, enviar um nome e algumas mensagens, e depois se desconectar."

- **Objetivo:** Automatizar o teste e a simulação de múltiplos clientes concorrentes no servidor.

- **Contribuição:**  Foi gerado um script que inicia vários processos do cliente. O script foi refinado para redirecionar a saída de cada cliente para arquivos de log separados (logs/), permitindo uma análise limpa e evitando a confusão de saídas no terminal.

### Prompt 7
> "Considerando a estrutura do projeto, atualize o makefile para incluir compilação do server e do cliente. Também deve incluir a execução do script simulador.sh."

- **Objetivo:** Simplificar o processo de compilação do código-fonte e permitir a execução do script de teste.

- **Contribuição:** Atualizou o Makefile para automatizar a compilçao de todos os arquivos do projeto e permitir a execução do test.