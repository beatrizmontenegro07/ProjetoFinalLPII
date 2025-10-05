#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h> 

#include "server.h"
#include "libtslog.h"

// variaveis globais
client* clients[MAX_CLIENTS]; // array para armazenar os clientes conectados
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex para garantir que o acesso ao array de clientes seja seguro
sem_t sem_clients; // semafaro para controlar a quantidade de clientes conectados
message_queue_t message_queue; //monitor para a fila de mensagens


// DEFINIÇÃO DAS FUNÇÕES DO MONITOR


void queue_init(message_queue_t *q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->not_empty, NULL);
}

void queue_push(message_queue_t *q, message_t msg) {
  pthread_mutex_lock(&q->mutex); // trava o muex para adicionar a mensagem na fila
  q->buffer[q->tail] = msg;
  q->tail = (q->tail + 1) % QUEUE_SIZE;
  q->count++;
  pthread_cond_signal(&q->not_empty); // sinaliza que há um novo item
  pthread_mutex_unlock(&q->mutex); // libera o mutex
}

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

void *broadcast_thread(void *arg) {
  while (1) {
    message_t msg = queue_pop(&message_queue); // retira a mensagem da fila
    broadcast_message(msg.sender_socket, msg.content); // realiza o broadcast
  }
  return NULL;
}

// DEFINIÇÃO DAS FUNÇÕES DO SERVIDOR

void server_start() {
    // Inicializa o logger
  if (tslog_init("logs/server_log.txt") != 0) {
    perror("Falha ao inicializar o logger");
    exit(EXIT_FAILURE);
  }

  tslog_log(INFO, "Iniciando o servidor...");

  if (sem_init(&sem_clients, 0, MAX_CLIENTS) != 0) { // inicialização do semáfaro com a quantidade maxima de clientes
    tslog_log(ERROR, "Falha ao inicializar o semáforo.");  // caso der erro, registra no logger
    exit(EXIT_FAILURE);
  }

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
  
  queue_init(&message_queue); // Inicializa a fila

  //cria a thread de broadcast
  pthread_t broadcast_t;
  pthread_create(&broadcast_t, NULL, &broadcast_thread, NULL);

  char log_msg[100];
  sprintf(log_msg, "Servidor escutando na porta %d", SERVER_PORT);
  tslog_log(INFO, log_msg);
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

    // aloca memória para a struct do novo cliente
    client *new_client = (client*)malloc(sizeof(client));
    new_client->address = client_address;
    new_client->socket_fd = client_socket_fd;
    new_client->is_active = 1;

    // cria uma thread para lidar com o cliente
    pthread_t thread_client;
    pthread_create(&thread_client, NULL, &handle_client, (void*)new_client);
  }

  // realiza a limpeza
  close(server_socket_fd);
  tslog_destroy();
  sem_destroy(&sem_clients);
}

void *handle_client(void *arg) {
  client *cli = (client*)arg;
  char buffer[BUFFER_SIZE];
  char message[BUFFER_SIZE + NAME_SIZE + 4];
  char log_message[BUFFER_SIZE + NAME_SIZE + 4];
  char log_msg[100];
  
  // recebe o nome do cliente
  int name_len = recv(cli->socket_fd, cli->nome, NAME_SIZE - 1, 0);
  if (name_len <= 0) {
    tslog_log(WARNING, "Não foi possível obter o nome do cliente.");
    close(cli->socket_fd);
    free(cli);
    return NULL;
  }

  cli->nome[name_len] = '\0';

  // adiciona o cliente à lista de clientes ativos
  add_client(cli);

  // Log e mensagem de broadcast da entrada do novo cliente
  sprintf(log_msg, "Cliente '%s' conectado de %s:%d.", cli->nome, inet_ntoa(cli->address.sin_addr), ntohs(cli->address.sin_port));
  tslog_log(INFO, log_msg);
  printf(">> %s entrou no chat.\n", cli->nome);
  sprintf(message, ">> %s entrou no chat.\n", cli->nome);
  broadcast_message(cli->socket_fd, message);

  // loop para receber as mensagens do cliente
  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    int receive_len = recv(cli->socket_fd, buffer, BUFFER_SIZE - 1, 0);

    if (receive_len > 0) {
      buffer[receive_len] = '\0';
      // monta a mensagem junto ao nome do cliente que a enviou
      sprintf(message, "[%s]: %s", cli->nome, buffer);
      printf("%s", message);

      strcpy(log_message, message);
      log_message[strcspn(log_message, "\r\n")] = '\0';
      tslog_log(INFO, log_message); // registra a mensagem enviada no log

      // adiciona a mensagem na fila
      message_t msg_queue;
      strncpy(msg_queue.content, message, sizeof(msg_queue.content));
      msg_queue.sender_socket = cli->socket_fd;
      queue_push(&message_queue, msg_queue);
    } else {
      // cliente desconectado
      remove_client(cli->socket_fd);
      sprintf(log_msg, "Cliente '%s' desconectado.", cli->nome);
      tslog_log(INFO, log_msg);
      printf(">> %s saiu do chat.\n", cli->nome);
      sprintf(message, ">> %s saiu do chat.\n", cli->nome);
      broadcast_message(cli->socket_fd, message);
      
      close(cli->socket_fd);
      free(cli);
      sem_post(&sem_clients); // libera um slot já que o cliente se desconectou
      break;
    }
  }
  
  pthread_detach(pthread_self());
  return NULL;
}


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