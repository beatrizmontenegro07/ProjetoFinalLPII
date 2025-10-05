# Mapeamento de Requisitos vs. Código
Este documento detalha como cada requisito obrigatório do projeto foi atendido, apontando para os arquivos e trechos de código relevantes.

## 1. Threads
O requisito de concorrência com pthreads é atendido tanto no servidor quanto no cliente.

- **Servidor**: No arquivo `server.c`, a função `server_start()` cria uma thread dedicada para cada cliente aceito com `pthread_create`. Além disso, uma thread para o broadcast de mensagens (broadcast_thread_func) é iniciada para gerenciar a fila de mensagens.

```bash
void server_start() {
   //...

  queue_init(&message_queue); // Inicializa a fila

  //cria a thread de broadcast
  pthread_t broadcast_t;
  pthread_create(&broadcast_t, NULL, &broadcast_thread, NULL);

  //...

  // loop para aceitar novas conexões
  while (1) {

    //...

    // cria uma thread para lidar com o cliente
    pthread_t thread_client;
    pthread_create(&thread_client, NULL, &handle_client, (void*)new_client);

    //...
  }

  //...
```

- **Cliente**: No arquivo `main_cliente.c`, é criado uma thread secundária para receber mensagens, enquanto a thread principal cuida do envio.

```bash
int main(int argc, char *argv[]) {

  //...

  // Inicia a thread para receber mensagens
  pthread_t recv_thread;
  pthread_create(&recv_thread, NULL, receive_messages, (void*)&socket_fd);

  //...

  pthread_join(recv_thread, NULL);

  //...
  
```

## 2. Exclusão Mútua
A exclusão mútua é utilizada para proteger o acesso a recursos compartilhados em diferentes partes do sistema.

- **Lista de Clientes**: Em `server.c`, o array global client* clients[] é protegido pelo mutex pthread_mutex_t clients_mutex. Funções como `add_client`, `remove_client` e `broadcast_message` travam este mutex antes de manipular o array.

```bash
void add_client(client* new_client) {
  pthread_mutex_lock(&clients_mutex); // ativa a trava do mutex para modificar a lista de clientes de forma segura
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i] == NULL) {
      clients[i] = new_client; // adiciona o novo cliente ao array
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}
```

```bash
void remove_client(int socket_fd) {
  pthread_mutex_lock(&clients_mutex); // ativa a trava do mutex para modificar a lista de clientes de forma segura
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (clients[i] && clients[i]->socket_fd == socket_fd) {
      clients[i] = NULL; // remove o cliente do array (marca o slot como livre)
      break;
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}
```  

```bash
void broadcast_message(int sender_socket, const char *message) {
  pthread_mutex_lock(&clients_mutex); // ativa a trava do mutex para acessar a lista de clientes de forma segura
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    // envia a mensagem se o cliente existir, estiver ativo e não for o remetente
    if (clients[i] && clients[i]->is_active && clients[i]->socket_fd != sender_socket) {
      send(clients[i]->socket_fd, message, strlen(message), 0);
    }
  }
  pthread_mutex_unlock(&clients_mutex);
}
```  

- **Fila de Mensagens**: A estrutura `message_queue_t` (Monitor) definida em `server.h` contém um pthread_mutex_t para proteger o buffer da fila durante as operações de `queue_push()` e `queue_pop()`.

```bash
typedef struct {
  message_t buffer[QUEUE_SIZE];
  int head, tail, count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty; // variavel conticional para sinalizar se a fila ta vazia ou não
} message_queue_t;
``` 

- **Arquivo de Log**: Em `libtslog.c`, a escrita no arquivo de log é protegida pelo mutex `pthread_mutex_t log_mutex` dentro da função `tslog_log`. Também é usado mutex ao fechar o arquivo de log na função `tslog_destroy` para garantir que ao fechar o arquivo ainda não tenha nenhuma operação de escrita sendo executada.

```bash
void tslog_log(LogLevel level, const char *message) {
  //...
  
  // trava o mutex para garantir a exclusão mútua na seção crítica
  pthread_mutex_lock(&log_mutex);

  // obtem o horário e a data atual 
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char time_str[20];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

  // mapeia o nível do log para o formato string
  char *level_str;
  switch (level) {
    case INFO: level_str = "INFO"; break;
    case WARNING: level_str = "WARNING"; break;
    case ERROR: level_str = "ERROR"; break;
    default: level_str = "UNKNOWN"; break;
  }

  // escreve no arquivo no formato [<DATA>] [<NÍVEL>] - <MENSSAGEM>
  fprintf(log_file, "[%s] [%s] - %s\n", time_str, level_str, message);
  
  // Garante que a mensagem seja escrita imediatamente no disco
  fflush(log_file);

  // libera o mutex
  pthread_mutex_unlock(&log_mutex);
}

void tslog_destroy() {
  if (log_file != NULL) {
    // trava o mutex 
    pthread_mutex_lock(&log_mutex);
    fclose(log_file);
    log_file = NULL;
    pthread_mutex_unlock(&log_mutex);

    pthread_mutex_destroy(&log_mutex); // destrói o mutex
  }
}
``` 

## 3. Semáforos e Variáveis de Condição
Ambos os mecanismos de sincronização foram implementados no servidor.

- **Semáforos**: Para controlar o número de clientes, foi utilizado o `semáforo sem_t sem_clients`. Ele é inicializado em `server_start()` com `sem_init()`, a thread principal espera em `sem_wait()` antes de aceitar uma nova conexão, e `sem_post()` é chamado em `handle_client` quando um cliente se desconecta, liberando um slot.

```bash
void server_start() {

  //...

  if (sem_init(&sem_clients, 0, MAX_CLIENTS) != 0) { // inicialização do semáfaro com a quantidade maxima de clientes
    tslog_log(ERROR, "Falha ao inicializar o semáforo.");  // caso der erro, registra no logger
    exit(EXIT_FAILURE);
  }

  //...

  // loop para aceitar novas conexões
  while (1) {

    sem_wait(&sem_clients); //bloqueia se o número de clientes já tiver antigido o máximo permitido
    
    //...

    if (client_socket_fd < 0) {
      tslog_log(WARNING, "Falha ao aceitar conexão.");
      sem_post(&sem_clients); // libera um slot se o accept falhar
      continue;
    }

    // ...
  }

  // realiza a limpeza
  close(server_socket_fd);
  tslog_destroy();
  sem_destroy(&sem_clients);
}
``` 

- **Variáveis de Condição**: A estrutura `message_queue_t` contém a variável de condição `pthread_cond_t not_empty`. A thread consumidora (broadcast_thread_func) espera nesta variável com `pthread_cond_wait()` se a fila estiver vazia, e as threads produtoras (handle_client) a sinalizam com `pthread_cond_signal()` ao adicionarem uma nova mensagem.

```bash
typedef struct {
  message_t buffer[QUEUE_SIZE];
  int head, tail, count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty; // variavel conticional para sinalizar se a fila ta vazia ou não
} message_queue_t;
```   

```bash
void queue_push(message_queue_t *q, message_t msg) {
  pthread_mutex_lock(&q->mutex); // trava o muex para adicionar a mensagem na fila
  q->buffer[q->tail] = msg;
  q->tail = (q->tail + 1) % QUEUE_SIZE;
  q->count++;
  pthread_cond_signal(&q->not_empty); // sinaliza que há um novo item
  pthread_mutex_unlock(&q->mutex); // libera o mutex
}
```  

```bash
message_t queue_pop(message_queue_t *q) {
  pthread_mutex_lock(&q->mutex); // trava o muex para remover a mensagem na fila
  // se a fila estiver vazia, espera pelo sinal
  while (q->count == 0) {
    pthread_cond_wait(&q->not_empty, &q->mutex);
  }
  message_t msg = q->buffer[q->head];
  q->head = (q->head + 1) % QUEUE_SIZE;
  q->count--;
  pthread_mutex_unlock(&q->mutex); // libera o mutex
  return msg;
}
```  

## 4. Monitores
O padrão Monitor foi implementado para gerenciar o broadcast de mensagens de forma desacoplada e segura.

`Fila de mansagens`: A fila de mensagens é um monitor. A struct `message_queue_t` encapsula os dados (o buffer de mensagens) e as primitivas de sincronização (mutex e variável de condição). As funções `queue_init`, `queue_push` e `queue_pop` servem para interagir com este recurso compartilhado.

```bash
typedef struct {
  message_t buffer[QUEUE_SIZE];
  int head, tail, count;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty; // variavel conticional para sinalizar se a fila ta vazia ou não
} message_queue_t;
```   

## 5. Sockets
O uso de sockets é base da comunicação em rede do projeto.

- **Servidor**: Utiliza as chamadas socket, setsockopt, bind, listen e accept para configurar e aceitar conexões.

```bash
void server_start() {

  //... 

  int server_socket_fd;
  struct sockaddr_in server_address;
  
  // cria o socket do servidor
  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_fd < 0) {
    tslog_log(ERROR, "Falha ao criar o socket do servidor."); // caso der erro, registra no logger
    exit(EXIT_FAILURE);
  }
  
  // configura o socket para reutilizar o endereço
  int opt = 1;
  setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt falhou");
    tslog_log(ERROR, "Falha no setsockopt do socket."); // caso der erro, registra no logger
    close(server_socket_fd);
    exit(EXIT_FAILURE);
  }
  
  // configurações do endereço e da porta
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(SERVER_PORT);
  
  // associa o socket ao endereço e a porta
  if (bind(server_socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    tslog_log(ERROR, "Falha no bind do socket.");
    exit(EXIT_FAILURE);
  }
  
  // coloca o socket do servidor em modo de escuta
  if (listen(server_socket_fd, 10) < 0) {
    tslog_log(ERROR, "Falha no listen do socket.");
    exit(EXIT_FAILURE);
  }
  
  //...

  printf("Servidor aguardando por conexões...\n");

  // loop para aceitar novas conexões
  while (1) {

    sem_wait(&sem_clients); //bloqueia se o número de clientes já tiver antigido o máximo permitido
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_address, &client_len);

    if (client_socket_fd < 0) {
      tslog_log(WARNING, "Falha ao aceitar conexão.");
      sem_post(&sem_clients); // libera um slot se o accept falhar
      continue;
    }

    // ...
  }
}
```  

- **Cliente**: Utiliza socket e connect para estabelecer a conexão com o servidor.

```bash
int client_connect(const char *ip, int port) {
  int socket_fd;
  struct sockaddr_in server_address;

  // cria o socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    tslog_log(ERROR, "Falha ao criar o socket do cliente."); // caso der erro, registra no logger
    exit(EXIT_FAILURE);
  }

  // configura o endereço do servidor
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &server_address.sin_addr) <= 0) {
    tslog_log(ERROR, "Endereço IP inválido."); // caso der erro, registra no logger
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  // conecta o cliente ao servidor
  if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    tslog_log(ERROR, "Falha ao conectar o cliente ao servidor."); // caso der erro, registra no logger
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}
```   

## 6. Gerenciamento de Recursos
O projeto possui um gerenciamento para o uso de memória, descritores de arquivos e threads.

- **Memória**: O servidor aloca memória para a struct de cliente com `malloc()` e a libera com `free()` quando a conexão termina.

- **Descritores de Arquivos**: Sockets são sempre fechados com `close()` em rotinas de desconexão e encerramento.

- **Threads**: O servidor usa `pthread_detach()` para que os recursos da thread do cliente sejam liberados automaticamente. O cliente usa `pthread_join()` para garantir um encerramento sincronizado.

## 7. Tratamento de Erros
O retorno de chamadas de sistema críticas é verificado para garantir a robustez da aplicação.

- Funções como socket, bind, listen, connect, etc., têm seu valor de retorno verificado. Em caso de falha (retorno < 0), uma mensagem de erro é impressa na saída de erro padrão (stderr) ou registrada no log, e o programa é encerrado com exit(EXIT_FAILURE).

Exemplo: 

```bash
// cria o socket do servidor
  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_fd < 0) {
    tslog_log(ERROR, "Falha ao criar o socket do servidor."); // caso der erro, registra no logger
    exit(EXIT_FAILURE);
  }
```  

## 8. Logging Concorrente
O sistema usa a biblioteca libtslog pra registrar os logs.


- O servidor e o cliente chamam tslog_log() para registrar eventos importantes (início, conexões, erros, desconexões). A biblioteca libtslog garante que essas chamadas, vindas de múltiplas threads, não corrompam o arquivo de log, graças ao uso de um mutex interno.